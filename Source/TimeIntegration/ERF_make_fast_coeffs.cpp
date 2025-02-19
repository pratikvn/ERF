#include <AMReX.H>
#include <AMReX_MultiFab.H>
#include <IndexDefines.H>
#include <TimeIntegration.H>
#include <prob_common.H>
#include <TileNoZ.H>

using namespace amrex;

void make_fast_coeffs (int /*level*/,
                       BoxArray& grids_to_evolve,
                       MultiFab& fast_coeffs,
                       Vector<MultiFab>& S_stage_data,                 // S_bar = S^n, S^* or S^**
                       const MultiFab& S_stage_prim,
                       const MultiFab& pi_stage,                       // Exner function evaluted at least stage
                       const amrex::Geometry geom,
                       const SolverChoice& solverChoice,
                       std::unique_ptr<MultiFab>& detJ_cc,
                       const MultiFab* r0, const MultiFab* pi0,
                       amrex::Real dtau)
{
    BL_PROFILE_VAR("make_fast_coeffs()",make_fast_coeffs);

    // beta_s = -1.0 : fully explicit
    // beta_s =  1.0 : fully implicit
    Real beta_s = 0.1;
    Real beta_2 = 0.5 * (1.0 + beta_s);  // multiplies implicit terms

    Real c_v = solverChoice.c_p - R_d;

    bool l_use_terrain    = solverChoice.use_terrain;

    const Box domain(geom.Domain());
    const GpuArray<Real, AMREX_SPACEDIM> dxInv = geom.InvCellSizeArray();

    Real dzi = dxInv[2];

    MultiFab coeff_A_mf(fast_coeffs, amrex::make_alias, 0, 1);
    MultiFab coeff_B_mf(fast_coeffs, amrex::make_alias, 1, 1);
    MultiFab coeff_C_mf(fast_coeffs, amrex::make_alias, 2, 1);
    MultiFab coeff_P_mf(fast_coeffs, amrex::make_alias, 3, 1);
    MultiFab coeff_Q_mf(fast_coeffs, amrex::make_alias, 4, 1);


    // *************************************************************************
    // Set gravity as a vector
    const    Array<Real,AMREX_SPACEDIM> grav{0.0, 0.0, -solverChoice.gravity};
    const GpuArray<Real,AMREX_SPACEDIM> grav_gpu{grav[0], grav[1], grav[2]};

    // *************************************************************************
    // Define updates in the current RK stage
    // *************************************************************************
#ifdef _OPENMP
#pragma omp parallel if (amrex::Gpu::notInLaunchRegion())
#endif
    {

    for ( MFIter mfi(S_stage_data[IntVar::cons],TileNoZ()); mfi.isValid(); ++mfi)
    {
        const Box& valid_bx = grids_to_evolve[mfi.index()];

        // Construct intersection of current tilebox and valid region for updating
        Box bx = mfi.tilebox() & valid_bx;

        Box tbz = surroundingNodes(bx,2);

        const Array4<const Real> & stage_cons = S_stage_data[IntVar::cons].const_array(mfi);
        const Array4<const Real> & prim       = S_stage_prim.const_array(mfi);

        const Array4<const Real>& detJ   = l_use_terrain ?   detJ_cc->const_array(mfi) : Array4<const Real>{};

        const Array4<const Real>& r0_ca       = r0->const_array(mfi);
        const Array4<const Real>& pi0_ca      = pi0->const_array(mfi); const Array4<const Real>& pi_stage_ca = pi_stage.const_array(mfi);

        FArrayBox gam_fab; gam_fab.resize(surroundingNodes(bx,2),1);
        Elixir gEli = gam_fab.elixir();

        auto const& coeffA_a  = coeff_A_mf.array(mfi);
        auto const& coeffB_a  = coeff_B_mf.array(mfi);
        auto const& coeffC_a  = coeff_C_mf.array(mfi);
        auto const& coeffP_a  = coeff_P_mf.array(mfi);
        auto const& coeffQ_a  = coeff_Q_mf.array(mfi);
        auto const&    gam_a  = gam_fab.array();

        // *********************************************************************
        // *********************************************************************
        // *********************************************************************

        Box bx_shrunk_in_k = bx;
        int klo = tbz.smallEnd(2);
        int khi = tbz.bigEnd(2);
        bx_shrunk_in_k.setSmall(2,klo+1);
        bx_shrunk_in_k.setBig(2,khi-1);

        // Note that the notes use "g" to mean the magnitude of gravity, so it is positive
        // We set grav_gpu[2] to be the vector component which is negative
        // We define halfg to match the notes (which is why we take the absolute value)
        Real halfg = std::abs(0.5 * grav_gpu[2]);

        //Note we don't act on the bottom or top boundaries of the domain
        if (l_use_terrain)
        {
            ParallelFor(bx_shrunk_in_k, [=] AMREX_GPU_DEVICE (int i, int j, int k)
            {
                Real rhobar_lo, rhobar_hi, pibar_lo, pibar_hi;
                rhobar_lo =  r0_ca(i,j,k-1);
                rhobar_hi =  r0_ca(i,j,k  );
                 pibar_lo = pi0_ca(i,j,k-1);
                 pibar_hi = pi0_ca(i,j,k  );

                 Real pi_lo = pi_stage_ca(i,j,k-1,0);
                 Real pi_hi = pi_stage_ca(i,j,k  ,0);
                 Real pi_c =  0.5 * (pi_lo + pi_hi);

                 Real     detJ_on_kface = 0.5 * (detJ(i,j,k) + detJ(i,j,k-1));
                 Real inv_detJ_on_kface = 1. / detJ_on_kface;

                 Real coeff_P = -Gamma * R_d * pi_c * dzi * inv_detJ_on_kface
                               +  halfg * R_d * rhobar_hi * pi_hi  /
                               (  c_v * pibar_hi * stage_cons(i,j,k,RhoTheta_comp) );

                 Real coeff_Q =  Gamma * R_d * pi_c * dzi * inv_detJ_on_kface
                               + halfg * R_d * rhobar_lo * pi_lo  /
                               ( c_v  * pibar_lo * stage_cons(i,j,k-1,RhoTheta_comp) );

                 coeffP_a(i,j,k) = coeff_P;
                 coeffQ_a(i,j,k) = coeff_Q;

#if defined(ERF_USE_MOISTURE)
                Real q = 0.5 * ( prim(i,j,k,PrimQt_comp) + prim(i,j,k-1,PrimQt_comp)
                                +prim(i,j,k,PrimQp_comp) + prim(i,j,k-1,PrimQp_comp) );
                coeff_P /= (1.0 + q);
                coeff_Q /= (1.0 + q);
#elif defined(ERF_USE_WARM_NO_PRECIP)
                Real q = 0.5 * ( prim(i,j,k  ,PrimQv_comp) + prim(i,j,k  ,PrimQc_comp) +
                                 prim(i,j,k-1,PrimQv_comp) + prim(i,j,k-1,PrimQc_comp) );
                coeff_P /= (1.0 + q);
                coeff_Q /= (1.0 + q);
#endif

                Real theta_t_lo  = 0.5 * ( prim(i,j,k-2,PrimTheta_comp) + prim(i,j,k-1,PrimTheta_comp) );
                Real theta_t_mid = 0.5 * ( prim(i,j,k-1,PrimTheta_comp) + prim(i,j,k  ,PrimTheta_comp) );
                Real theta_t_hi  = 0.5 * ( prim(i,j,k  ,PrimTheta_comp) + prim(i,j,k+1,PrimTheta_comp) );

                // LHS for tri-diagonal system
                Real D = dtau * dtau * beta_2 * beta_2 * dzi;
                coeffA_a(i,j,k) = D * ( halfg - coeff_Q * theta_t_lo );
                coeffC_a(i,j,k) = D * (-halfg + coeff_P * theta_t_hi );

                coeffB_a(i,j,k) = detJ_on_kface + D * (coeff_Q - coeff_P) * theta_t_mid;
            });

        } else {

            ParallelFor(bx_shrunk_in_k, [=] AMREX_GPU_DEVICE (int i, int j, int k)
            {
                Real rhobar_lo, rhobar_hi, pibar_lo, pibar_hi;
                rhobar_lo =  r0_ca(i,j,k-1);
                rhobar_hi =  r0_ca(i,j,k  );
                 pibar_lo = pi0_ca(i,j,k-1);
                 pibar_hi = pi0_ca(i,j,k  );

                 Real pi_lo = pi_stage_ca(i,j,k-1,0);
                 Real pi_hi = pi_stage_ca(i,j,k  ,0);
                 Real pi_c =  0.5 * (pi_lo + pi_hi);

                 Real coeff_P = -Gamma * R_d * pi_c * dzi
                              +  halfg * R_d * rhobar_hi * pi_hi  /
                              (  c_v * pibar_hi * stage_cons(i,j,k,RhoTheta_comp) );

                 Real coeff_Q = Gamma * R_d * pi_c * dzi
                              + halfg * R_d * rhobar_lo * pi_lo  /
                              ( c_v  * pibar_lo * stage_cons(i,j,k-1,RhoTheta_comp) );

                 coeffP_a(i,j,k) = coeff_P;
                 coeffQ_a(i,j,k) = coeff_Q;

#if defined(ERF_USE_MOISTURE)
                Real q = 0.5 * ( prim(i,j,k,PrimQt_comp) + prim(i,j,k-1,PrimQt_comp)
                                +prim(i,j,k,PrimQp_comp) + prim(i,j,k-1,PrimQp_comp) );
                coeff_P /= (1.0 + q);
                coeff_Q /= (1.0 + q);
#elif defined(ERF_USE_WARM_NO_PRECIP)
                Real q = 0.5 * ( prim(i,j,k  ,PrimQv_comp) + prim(i,j,k  ,PrimQc_comp) +
                                 prim(i,j,k-1,PrimQv_comp) + prim(i,j,k-1,PrimQc_comp) );
                coeff_P /= (1.0 + q);
                coeff_Q /= (1.0 + q);
#endif

                Real theta_t_lo  = 0.5 * ( prim(i,j,k-2,PrimTheta_comp) + prim(i,j,k-1,PrimTheta_comp) );
                Real theta_t_mid = 0.5 * ( prim(i,j,k-1,PrimTheta_comp) + prim(i,j,k  ,PrimTheta_comp) );
                Real theta_t_hi  = 0.5 * ( prim(i,j,k  ,PrimTheta_comp) + prim(i,j,k+1,PrimTheta_comp) );

                // LHS for tri-diagonal system
                Real D = dtau * dtau * beta_2 * beta_2 * dzi;
                coeffA_a(i,j,k) = D * ( halfg - coeff_Q * theta_t_lo );
                coeffC_a(i,j,k) = D * (-halfg + coeff_P * theta_t_hi );

                coeffB_a(i,j,k) = 1.0 + D * (coeff_Q - coeff_P) * theta_t_mid;
            });
        }

        amrex::Box b2d = tbz; // Copy constructor
        b2d.setRange(2,0);

        auto const hi = amrex::ubound(bx);

        {
        BL_PROFILE("make_coeffs_b2d_loop");
#ifdef AMREX_USE_GPU
        ParallelFor(b2d, [=] AMREX_GPU_DEVICE (int i, int j, int) {
          // w_0 = 0
          coeffA_a(i,j,0) =  0.0;
          coeffB_a(i,j,0) =  1.0;
          coeffC_a(i,j,0) =  0.0;

          // w_khi = 0
          coeffA_a(i,j,hi.z+1) =  0.0;
          coeffB_a(i,j,hi.z+1) =  1.0;
          coeffC_a(i,j,hi.z+1) =  0.0;

          // w = 0 at k = 0
          Real bet = coeffB_a(i,j,0);

          for (int k = 1; k <= hi.z+1; k++) {
              gam_a(i,j,k) = coeffC_a(i,j,k-1) / bet;
              bet = coeffB_a(i,j,k) - coeffA_a(i,j,k)*gam_a(i,j,k);
              coeffB_a(i,j,k) = bet;
          }
        });
#else
        auto const lo = amrex::lbound(bx);
        for (int j = lo.y; j <= hi.y; ++j) {
            AMREX_PRAGMA_SIMD
            for (int i = lo.x; i <= hi.x; ++i) {
                coeffA_a(i,j,0) =  0.0;
                coeffB_a(i,j,0) =  1.0;
                coeffC_a(i,j,0) =  0.0;
            }
        }
        for (int j = lo.y; j <= hi.y; ++j) {
            AMREX_PRAGMA_SIMD
            for (int i = lo.x; i <= hi.x; ++i) {
                coeffA_a(i,j,hi.z+1) =  0.0;
                coeffB_a(i,j,hi.z+1) =  1.0;
                coeffC_a(i,j,hi.z+1) =  0.0;
            }
        }
        for (int k = lo.z+1; k <= hi.z+1; ++k) {
            for (int j = lo.y; j <= hi.y; ++j) {
                AMREX_PRAGMA_SIMD
                for (int i = lo.x; i <= hi.x; ++i) {
                    gam_a(i,j,k) = coeffC_a(i,j,k-1) / coeffB_a(i,j,k-1);
                    Real bet = coeffB_a(i,j,k) - coeffA_a(i,j,k)*gam_a(i,j,k);
                    coeffB_a(i,j,k) = bet;
                }
            }
        }
#endif
        } // end profile

        // In the end we save the inverse of the diagonal (B) coefficient
        {
        BL_PROFILE("make_coeffs_invert");
            ParallelFor(bx_shrunk_in_k, [=] AMREX_GPU_DEVICE (int i, int j, int k)
            {
                coeffB_a(i,j,k) = 1.0 / coeffB_a(i,j,k);
            });
        } // end profile
    } // mfi
    } // omp
}

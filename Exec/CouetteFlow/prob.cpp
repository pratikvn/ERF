#include "prob.H"
#include "prob_common.H"

#include "IndexDefines.H"
#include "AMReX_ParmParse.H"
#include "AMReX_MultiFab.H"
#include "AMReX_Geometry.H"

using namespace amrex;

ProbParm parms;

void
erf_init_dens_hse(MultiFab& rho_hse,
                  std::unique_ptr<MultiFab>&,
                  std::unique_ptr<MultiFab>&,
                  amrex::Geometry const&)
{
    Real rho_0 = parms.rho_0;
#ifdef _OPENMP
#pragma omp parallel if (amrex::Gpu::notInLaunchRegion())
#endif
    for ( MFIter mfi(rho_hse,TilingIfNotGPU()); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.growntilebox(1);
        const Array4<Real> rho_hse_arr = rho_hse[mfi].array();
        amrex::ParallelFor(bx, [=] AMREX_GPU_DEVICE(int i, int j, int k) noexcept
        {
            rho_hse_arr(i,j,k) = rho_0;
        });
    }
}

void
erf_init_rayleigh(amrex::Vector<Real>& /*tau*/,
                  amrex::Vector<Real>& /*ubar*/,
                  amrex::Vector<Real>& /*vbar*/,
                  amrex::Vector<Real>& /*wbar*/,
                  amrex::Vector<Real>& /*thetabar*/,
                  amrex::Geometry      const& /*geom*/)
{
   amrex::Error("Should never get here for CouetteFlow problem");
}

void
init_custom_prob(
    const Box& bx,
    const Box& xbx,
    const Box& ybx,
    const Box& zbx,
    Array4<Real> const& state,
    Array4<Real> const& x_vel,
    Array4<Real> const& y_vel,
    Array4<Real> const& z_vel,
    Array4<Real> const&,
    Array4<Real> const&,
    Array4<Real const> const&,
    Array4<Real const> const&,
#if defined(ERF_USE_MOISTURE)
    Array4<Real      > const&,
    Array4<Real      > const&,
    Array4<Real      > const&,
#elif defined(ERF_USE_WARM_NO_PRECIP)
    Array4<Real      > const&,
    Array4<Real      > const&,
#endif
    amrex::GeometryData const& geomdata,
    Array4<Real const> const& /*mf_m*/,
    Array4<Real const> const& /*mf_u*/,
    Array4<Real const> const& /*mf_v*/,
    const SolverChoice&)
{
    amrex::ParallelFor(bx, [=, parms=parms] AMREX_GPU_DEVICE(int i, int j, int k) noexcept {

    // Set the density
    state(i, j, k, Rho_comp) = parms.rho_0;

    // Initial potential temperature
    state(i, j, k, RhoTheta_comp) = parms.rho_0 * parms.Theta_0;

    // Set scalar to 0
    state(i, j, k, RhoScalar_comp) = 0.0;

#if defined(ERF_USE_MOISTURE)
    state(i, j, k, RhoQt_comp) = 0.0;
    state(i, j, k, RhoQp_comp) = 0.0;
#elif defined(ERF_USE_WARM_NO_PRECIP)
    state(i, j, k, RhoQv_comp) = 0.0;
    state(i, j, k, RhoQc_comp) = 0.0;
#endif

  });

  amrex::ParallelFor(xbx, [=, parms=parms] AMREX_GPU_DEVICE(int i, int j, int k) noexcept {
    const auto *const prob_hi  = geomdata.ProbHi();
    const auto *const dx       = geomdata.CellSize();
    const Real z = (k + 0.5) * dx[2];
    x_vel(i, j, k) = parms.u_0 * z / prob_hi[2];
  });

  amrex::ParallelFor(ybx, [=, parms=parms] AMREX_GPU_DEVICE(int i, int j, int k) noexcept {
    const auto *const prob_hi  = geomdata.ProbHi();
    const auto *const dx       = geomdata.CellSize();
    const Real z = (k + 0.5) * dx[2];
    y_vel(i, j, k) = parms.v_0 * z / prob_hi[2];
  });

  amrex::ParallelFor(zbx, [=, parms=parms] AMREX_GPU_DEVICE(int i, int j, int k) noexcept {
    z_vel(i, j, k) = parms.w_0;
  });
}

void
init_custom_terrain (const Geometry& /*geom*/,
                           MultiFab& z_phys_nd,
                     const Real& /*time*/)
{
    // Number of ghost cells
    int ngrow = z_phys_nd.nGrow();

    for ( MFIter mfi(z_phys_nd, TilingIfNotGPU()); mfi.isValid(); ++mfi )
    {
        // Grown box with no z range
        amrex::Box xybx = mfi.growntilebox(ngrow);
        xybx.setRange(2,0);

        Array4<Real> z_arr = z_phys_nd.array(mfi);

        ParallelFor(xybx, [=] AMREX_GPU_DEVICE (int i, int j, int) {

            // Flat terrain with z = 0 at k = 0
            z_arr(i,j,0) = 0.;
        });
    }
}

void
amrex_probinit(
  const amrex_real* /*problo*/,
  const amrex_real* /*probhi*/)
{
  // Parse params
  amrex::ParmParse pp("prob");
  pp.query("rho_0", parms.rho_0);
  pp.query("T_0", parms.Theta_0);
  pp.query("u_0", parms.u_0);
  pp.query("v_0", parms.v_0);
  pp.query("w_0", parms.w_0);
}

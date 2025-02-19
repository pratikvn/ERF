#include <NumericalDiffusion.H>

using namespace amrex;

void
NumericalDiffusion (const Box& bx,
                    const int  start_comp,
                    const int  num_comp,
                    const Real dt,
                    const SolverChoice& solverChoice,
                    const Array4<const Real>& data,
                    const Array4<      Real>& rhs,
                    const Array4<const Real>& mf_x,
                    const Array4<const Real>& mf_y,
                    const bool avg_mf_x_y,
                    const bool avg_mf_y_x)
{
    BL_PROFILE_VAR("NumericalDiffusion()",NumericalDiffusion);

    // Average map factors to correct locations
    Box planebx(bx); planebx.setSmall(2,0); planebx.setBig(2,0);
    FArrayBox mf_x_bar; mf_x_bar.resize(planebx,1);
    FArrayBox mf_y_bar; mf_y_bar.resize(planebx,1);
    Elixir mfx_eli = mf_x_bar.elixir();
    Elixir mfy_eli = mf_y_bar.elixir();
    const Array4<Real>& mfx_arr  = mf_x_bar.array();
    const Array4<Real>& mfy_arr  = mf_y_bar.array();
    amrex::ParallelFor(planebx, [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        if (avg_mf_x_y) {
            mfx_arr(i,j,k) = 0.5 * ( mf_x(i,j-1,k) + mf_x(i,j,k) );
        } else {
            mfx_arr(i,j,k) = mf_x(i,j,k);
        }
        if (avg_mf_y_x) {
            mfy_arr(i,j,k) = 0.5 * ( mf_y(i-1,j,k) + mf_y(i,j,k) );
        } else {
            mfy_arr(i,j,k) = mf_y(i,j,k);
        }
    });

    // Capture diffusion coeff
    Real coeff6 = solverChoice.NumDiffCoeff / (2.0 * dt);

    // Compute 5th order derivative and augment RHS
    amrex::ParallelFor(bx, num_comp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int m) noexcept
    {
        int n = start_comp + m;
        Real xflux_lo = 10. * (data(i  ,j,k,n) - data(i-1,j,k,n))
                       - 5. * (data(i+1,j,k,n) - data(i-2,j,k,n))
                            + (data(i+2,j,k,n) - data(i-3,j,k,n));
        if ( (xflux_lo * (data(i,j,k,n) - data(i-1,j,k,n)) ) > 0.) xflux_lo = 0.;
        Real xflux_hi = 10. * (data(i+1,j,k,n) - data(i  ,j,k,n))
                       - 5. * (data(i+2,j,k,n) - data(i-1,j,k,n))
                            + (data(i+3,j,k,n) - data(i-2,j,k,n));
        if ( (xflux_hi * (data(i+1,j,k,n) - data(i,j,k,n)) ) > 0.) xflux_hi = 0.;
        Real yflux_lo = 10. * (data(i,j  ,k,n) - data(i,j-1,k,n))
                       - 5. * (data(i,j+1,k,n) - data(i,j-2,k,n))
                            + (data(i,j+2,k,n) - data(i,j-3,k,n));
        if ( (yflux_lo * (data(i,j,k,n) - data(i,j-1,k,n)) ) > 0.) yflux_lo = 0.;
        Real yflux_hi = 10. * (data(i,j+1,k,n) - data(i,j  ,k,n))
                       - 5. * (data(i,j+2,k,n) - data(i,j-1,k,n))
                            + (data(i,j+3,k,n) - data(i,j-2,k,n));
        if ( (yflux_hi * (data(i,j+1,k,n) - data(i,j,k,n)) ) > 0.) yflux_hi = 0.;
        rhs(i,j,k,n) += coeff6 * ( (xflux_hi - xflux_lo) * mfx_arr(i,j,0)
                                 + (yflux_hi - yflux_lo) * mfy_arr(i,j,0) );
    });
}

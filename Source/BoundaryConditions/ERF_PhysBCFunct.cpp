#include "AMReX_PhysBCFunct.H"
#include "IndexDefines.H"
#include <ERF_PhysBCFunct.H>

using namespace amrex;

//
// mf is the multifab to be filled
// icomp is the index into the MultiFab -- if cell-centered this can be any value
//       from 0 to NVAR-1, if face-centered this must be 0
// ncomp is the number of components -- if cell-centered (var_idx = 0) this can be any value
//       from 1 to NVAR as long as icomp+ncomp <= NVAR-1.  If face-centered this
//       must be 1
// nghost is how many ghost cells to be filled
// time is the time at which the data should be filled
// bccomp is the index into both domain_bcs_type_bcr and bc_extdir_vals for icomp = 0  --
//     so this follows the BCVars enum
//
void ERFPhysBCFunct::operator() (const Vector<MultiFab*>& mfs, int icomp_cons, int ncomp_cons,
                                 IntVect const& nghost_cons, IntVect const& nghost_vels, Real time,
                                 std::string& init_type, bool cons_only)
{
    BL_PROFILE("ERFPhysBCFunct::()");

#ifndef ERF_USE_NETCDF
    amrex::ignore_unused(init_type);
#endif

    if (m_geom.isAllPeriodic()) return;

    const auto& domain = m_geom.Domain();
    const auto dxInv   = m_geom.InvCellSizeArray();

    // Create a grown domain box containing valid + periodic cells
    Box gdomain  = domain;
    for (int i = 0; i < AMREX_SPACEDIM; ++i) {
        if (m_geom.isPeriodic(i)) {
            gdomain.grow(i, nghost_cons[i]);
        }
    }

    Box gdomainx = surroundingNodes(domain,0);
    for (int i = 0; i < AMREX_SPACEDIM; ++i) {
        if (m_geom.isPeriodic(i)) {
            gdomainx.grow(i, nghost_vels[i]);
        }
    }

    Box gdomainy = surroundingNodes(domain,1);
    for (int i = 0; i < AMREX_SPACEDIM; ++i) {
        if (m_geom.isPeriodic(i)) {
            gdomainy.grow(i, nghost_vels[i]);
        }
    }

#ifdef AMREX_USE_OMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
    {
        for (MFIter mfi(*mfs[Vars::cons],false); mfi.isValid(); ++mfi)
        {
            //
            // This is the box we pass to the different routines
            // NOTE -- this is the full grid NOT the tile box
            //
            Box bx  = mfi.validbox();

            //
            // These are the boxes we use to test on relative to the domain
            //
            Box cbx = bx;                     cbx.grow(nghost_cons);
            Box xbx = surroundingNodes(bx,0); xbx.grow(nghost_vels);
            Box ybx = surroundingNodes(bx,1); ybx.grow(nghost_vels);
            Box zbx = surroundingNodes(bx,2); zbx.grow(0,nghost_vels[0]);
                                              zbx.grow(1,nghost_vels[1]);

            const Array4<      Real> cons_arr = mfs[Vars::cons]->array(mfi);;
            const Array4<      Real> velx_arr = mfs[Vars::xvel]->array(mfi);;
            const Array4<      Real> vely_arr = mfs[Vars::yvel]->array(mfi);;
            const Array4<      Real> velz_arr = mfs[Vars::zvel]->array(mfi);;
                  Array4<const Real> z_nd_arr;

            if (m_z_phys_nd)
            {
                z_nd_arr = m_z_phys_nd->const_array(mfi);
            }

            if (init_type != "real")
            {

                //! If there are cells not in the valid + periodic grown box
                //! we need to fill them here
                if (!gdomain.contains(cbx))
                {
                    int bccomp = BCVars::cons_bc;
                    impose_lateral_cons_bcs(cons_arr,cbx,domain,icomp_cons,ncomp_cons,time,bccomp);
                }

                if (!cons_only)
                {
                    if (!gdomainx.contains(xbx))
                    {
                        impose_lateral_xvel_bcs(velx_arr,xbx,domain,time,BCVars::xvel_bc);
                    }

                    if (!gdomainy.contains(ybx))
                    {
                        impose_lateral_yvel_bcs(vely_arr,ybx,domain,time,BCVars::yvel_bc);
                    }

                    impose_lateral_zvel_bcs(velz_arr,zbx,domain,z_nd_arr,dxInv,time,BCVars::zvel_bc);
                } // !cons_only
            } // init_type != "real"

            // Every grid touches the bottom and top domain boundary so we call the vertical bcs
            //       for every box -- and we need to call these even if init_type == real
            {
            int bccomp = BCVars::cons_bc;
            impose_vertical_cons_bcs(cons_arr,cbx,domain,z_nd_arr,dxInv,
                                     icomp_cons,ncomp_cons,time,bccomp);
            }

            if (!cons_only) {
                impose_vertical_xvel_bcs(velx_arr,xbx,domain,z_nd_arr,dxInv,
                                         time,BCVars::xvel_bc);
                impose_vertical_yvel_bcs(vely_arr,ybx,domain,z_nd_arr,dxInv,
                                         time,BCVars::yvel_bc);
                impose_vertical_zvel_bcs(velz_arr,zbx,domain,z_nd_arr,dxInv,time,
                                         BCVars::xvel_bc, BCVars::yvel_bc, BCVars::zvel_bc,
                                         m_terrain_type);
            } // !cons_only

        } // MFIter
    } // OpenMP
} // operator()

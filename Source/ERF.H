#ifndef ERF_H_
#define ERF_H_

#include <string>
#include <limits>
#include <memory>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <AMReX_AmrCore.H>
#include <AMReX_BCRec.H>
#include <AMReX_InterpFaceRegister.H>

#include <AMReX_ParallelDescriptor.H>
#include <AMReX_ParmParse.H>
#include <AMReX_MultiFabUtil.H>
#include <AMReX_FillPatchUtil.H>
#include <AMReX_VisMF.H>
#include <AMReX_PhysBCFunct.H>
#include <AMReX_FluxRegister.H>
#include <AMReX_ErrorList.H>

#ifdef AMREX_MEM_PROFILING
#include <AMReX_MemProfiler.H>
#endif

#include <ERF_Math.H>
#include <IndexDefines.H>
#include <TimeInterpolatedData.H>
#include <DataStruct.H>
#include <InputSoundingData.H>
#include <ABLMost.H>
#include <Derive.H>
#include <ERF_ReadBndryPlanes.H>
#include <ERF_WriteBndryPlanes.H>
#include <ERF_MRI.H>
#include <ERF_PhysBCFunct.H>

#ifdef ERF_USE_MOISTURE
#include "Microphysics.H"
#endif

#ifdef ERF_USE_NETCDF
#include "NCWpsFile.H"
#endif

#include <iostream>

#ifdef AMREX_LAZY
#include <AMReX_Lazy.H>
#endif

#ifdef ERF_USE_MULTIBLOCK
class MultiBlockContainer;
#endif

namespace InterpType {
    enum {
        PCInterp = 0,
        NodeBilinear,
        CellConservativeLinear,
        CellBilinear,
        CellQuadratic,
        CellConservativeProtected,
        CellConservativeQuartic
    };
}

template<typename V, typename T>
bool containerHasElement(const V& iterable, const T& query) {
    return std::find(iterable.begin(), iterable.end(), query) != iterable.end();
}

class ERF
    : public amrex::AmrCore
{
public:

    ////////////////
    // public member functions

    // constructor - reads in parameters from inputs file
    //             - sizes multilevel arrays and data structures
    ERF ();
    ~ERF() override;

    // Declare a default move constructor so we ensure the destructor is
    // not called when we return an object of this class by value
    ERF(ERF&&)  noexcept = default;

    // Declare a default move assignment operator
    ERF& operator=(ERF&& other)  noexcept = default;

    // Delete the copy constructor
    ERF(const ERF& other) = delete;
    //
    // Delete the copy assignment operator
    ERF& operator=(const ERF& other) = delete;

    // Advance solution to final time
    void Evolve ();

    // Tag cells for refinement
    void ErrorEst (int lev, amrex::TagBoxArray& tags, amrex::Real time, int ngrow) override;

    // Initialize multilevel data
    void InitData ();

#ifdef ERF_USE_POISSON_SOLVE
    // Project the initial velocities to be divergence-free
    void project_initial_velocities ();

    // Define the projection bc's based on the domain bc types
    amrex::Array<amrex::LinOpBCType,AMREX_SPACEDIM>
      get_projection_bc (amrex::Orientation::Side side) const noexcept;
#endif

    // Init (NOT restart or regrid)
    void init_only (int lev, amrex::Real time);

    // Restart
    void restart ();

    // Called after every level 0 timestep
    void post_timestep (int nstep, amrex::Real time, amrex::Real dt_lev);

    // Diagnostics
    void sum_integrated_quantities(amrex::Real time);
    void write_1D_profiles(amrex::Real time);

    void sample_points(int lev, amrex::Real time, amrex::IntVect cell, amrex::MultiFab& mf);
    void sample_lines (int lev, amrex::Real time, amrex::IntVect cell, amrex::MultiFab& mf);

    void derive_diag_profiles(amrex::Gpu::HostVector<amrex::Real>& h_avg_u   , amrex::Gpu::HostVector<amrex::Real>& h_avg_v  ,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_w   , amrex::Gpu::HostVector<amrex::Real>& h_avg_rho,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_th  , amrex::Gpu::HostVector<amrex::Real>& h_avg_ksgs,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_uu  , amrex::Gpu::HostVector<amrex::Real>& h_avg_uv ,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_uw,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_vv  , amrex::Gpu::HostVector<amrex::Real>& h_avg_vw ,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_ww,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_uth , amrex::Gpu::HostVector<amrex::Real>& h_avg_vth,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_wth, amrex::Gpu::HostVector<amrex::Real>& h_avg_thth,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_k,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_ku, amrex::Gpu::HostVector<amrex::Real>& h_avg_kv,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_kw,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_p,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_pu, amrex::Gpu::HostVector<amrex::Real>& h_avg_pv,
                              amrex::Gpu::HostVector<amrex::Real>& h_avg_pw);

    void derive_stress_profiles(amrex::Gpu::HostVector<amrex::Real>& h_avg_tau11, amrex::Gpu::HostVector<amrex::Real>& h_avg_tau12,
                                amrex::Gpu::HostVector<amrex::Real>& h_avg_tau13, amrex::Gpu::HostVector<amrex::Real>& h_avg_tau22,
                                amrex::Gpu::HostVector<amrex::Real>& h_avg_tau23, amrex::Gpu::HostVector<amrex::Real>& h_avg_tau33,
                                amrex::Gpu::HostVector<amrex::Real>& h_avg_hfx3,  amrex::Gpu::HostVector<amrex::Real>& h_avg_diss);

    // Perform the volume-weighted sum
    amrex::Real
    volWgtSumMF(int lev,
      const amrex::MultiFab& mf, int comp, bool local, bool finemask);

    // Decide if it is time to take an action
    static bool is_it_time_for_action(int nstep, amrex::Real time, amrex::Real dt,
                               int action_interval, amrex::Real action_per);

    // Make a new level using provided BoxArray and DistributionMapping and
    // fill with interpolated coarse level data.
    // overrides the pure virtual function in AmrCore
    void MakeNewLevelFromCoarse (int lev, amrex::Real time, const amrex::BoxArray& ba,
                                 const amrex::DistributionMapping& dm) override;

    // Remake an existing level using provided BoxArray and DistributionMapping and
    // fill with existing fine and coarse data.
    // overrides the pure virtual function in AmrCore
    void RemakeLevel (int lev, amrex::Real time, const amrex::BoxArray& ba,
                      const amrex::DistributionMapping& dm) override;

    // Delete level data
    // overrides the pure virtual function in AmrCore
    void ClearLevel (int lev) override;

    // Make a new level from scratch using provided BoxArray and DistributionMapping.
    // Only used during initialization.
    // overrides the pure virtual function in AmrCore
    void MakeNewLevelFromScratch (int lev, amrex::Real time, const amrex::BoxArray& ba,
                      const amrex::DistributionMapping& dm) override;

    // compute dt from CFL considerations
    amrex::Real estTimeStep (int lev, long& dt_fast_ratio) const;

    // Interface for advancing the data at one level by one "slow" timestep
    void erf_advance(int level,
                      amrex::MultiFab& cons_old,  amrex::MultiFab& cons_new,
                      amrex::MultiFab& xvel_old,  amrex::MultiFab& yvel_old,  amrex::MultiFab& zvel_old,
                      amrex::MultiFab& xvel_new,  amrex::MultiFab& yvel_new,  amrex::MultiFab& zvel_new,
                      amrex::MultiFab& xmom_old,  amrex::MultiFab& ymom_old,  amrex::MultiFab& zmom_old,
                      amrex::MultiFab& xmom_new,  amrex::MultiFab& ymom_new,  amrex::MultiFab& zmom_new,
                      amrex::MultiFab& xmom_crse, amrex::MultiFab& ymom_crse, amrex::MultiFab& zmom_crse,
                      amrex::MultiFab& source,    amrex::MultiFab& buoyancy,
#if defined(ERF_USE_MOISTURE)
                      const amrex::MultiFab& qvapor, const amrex::MultiFab& qcloud, const amrex::MultiFab& qice,
#elif defined(ERF_USE_WARM_NO_PRECIP)
#endif
                      amrex::Geometry fine_geom,
                      amrex::Real dt, amrex::Real time,
                      amrex::InterpFaceRegister* ifr);

    amrex::MultiFab& build_fine_mask(int lev);

    void MakeHorizontalAverages();
    void MakeDiagnosticAverage(amrex::Vector<amrex::Real>& h_havg, amrex::MultiFab& S, int n);
    void derive_upwp          (amrex::Vector<amrex::Real>& h_havg);

    // write plotfile to disk
    void WritePlotFile  (int which, amrex::Vector<std::string> plot_var_names);

    void WriteMultiLevelPlotfileWithTerrain (const std::string &plotfilename,
                                             int nlevels,
                                             const amrex::Vector<const amrex::MultiFab*> &mf,
                                             const amrex::Vector<const amrex::MultiFab*> &mf_nd,
                                             const amrex::Vector<std::string> &varnames,
                                             amrex::Real time,
                                             const amrex::Vector<int> &level_steps,
                                             const std::string &versionName = "HyperCLaw-V1.1",
                                             const std::string &levelPrefix = "Level_",
                                             const std::string &mfPrefix = "Cell",
                                             const amrex::Vector<std::string>& extra_dirs = amrex::Vector<std::string>()) const;


    void WriteGenericPlotfileHeaderWithTerrain (std::ostream &HeaderFile,
                                                int nlevels,
                                                const amrex::Vector<amrex::BoxArray> &bArray,
                                                const amrex::Vector<std::string> &varnames,
                                                amrex::Real time,
                                                const amrex::Vector<int> &level_steps,
                                                const std::string &versionName,
                                                const std::string &levelPrefix,
                                                const std::string &mfPrefix) const;

    void erf_enforce_hse(int lev,
                         amrex::MultiFab& dens, amrex::MultiFab& pres, amrex::MultiFab& pi,
                         std::unique_ptr<amrex::MultiFab>& z_cc,
                         std::unique_ptr<amrex::MultiFab>& z_nd);

#ifdef ERF_USE_NETCDF
    //! Write a timestep to 1D vertical column output for coupling
    void writeToNCColumnFile(const int lev,
                             const std::string& colfile_name, const amrex::Real xloc, const amrex::Real yloc,
                             const amrex::Real time);
#endif //ERF_USE_NETCDF

    void init_from_input_sounding(int lev);

#ifdef ERF_USE_MULTIBLOCK
    // constructor used when ERF is created by a multiblock driver
    // calls AmrCore constructor -> AmrMesh constructor
    ERF (const amrex::RealBox& rb, int max_level_in,
         const amrex::Vector<int>& n_cell_in, int coord,
         const amrex::Vector<amrex::IntVect>& ref_ratios,
         const amrex::Array<int,AMREX_SPACEDIM>& is_per,
         std::string prefix);

    // Advance a block specified number of time steps
    void Evolve_MB (int MBstep, int max_block_step);

    // Set parmparse prefix for MultiBlock
    void SetParmParsePrefix(std::string name) { pp_prefix = name; }

    // Set 'this' from multiblock container
    void SetMultiBlockPointer(MultiBlockContainer *mbc) { m_mbc = mbc; }

    // Public data copy for MB
    std::vector<amrex::Box> domain_p;
    MultiBlockContainer *m_mbc = nullptr;
    amrex::Vector<amrex::Vector<amrex::MultiFab> > vars_new;
    amrex::Vector<amrex::Vector<amrex::MultiFab> > vars_old;
#endif
    std::string pp_prefix {"erf"};

    void fill_from_bndryregs (const amrex::Vector<amrex::MultiFab*>& mfs,
                              amrex::Real time);

#ifdef ERF_USE_NETCDF
    void fill_from_wrfbdy (const amrex::Vector<amrex::MultiFab*>& mfs,
                           const amrex::Real time);
#endif

private:

    ///////////////////////////
    // private member functions
    ///////////////////////////

    // read in some parameters from inputs file
    void ReadParameters();

    // set covered coarse cells to be the average of overlying fine cells
    void AverageDown ();

    void define_grids_to_evolve (int lev); // NOLINT

    void init1DArrays();

    void init_bcs();

    void init_custom(int lev);

    void initialize_integrator(int lev, amrex::MultiFab& cons_mf, amrex::MultiFab& vel_mf);

#ifdef ERF_USE_NETCDF
    void init_from_wrfinput(int lev);
    void init_from_metgrid(int lev);
#endif // ERF_USE_NETCDF

    // more flexible version of AverageDown() that lets you average down across multiple levels
    void AverageDownTo (int crse_lev); // NOLINT

    // compute a new multifab by copying in phi from valid region and filling ghost cells
    // works for single level and 2-level cases (fill fine grid ghost by interpolating from coarse)
    void FillPatch (int lev, amrex::Real time, const amrex::Vector<amrex::MultiFab*>& mf);

    // compute new multifabs by copying in data from valid region and filling ghost cells
    // works for single level and 2-level cases (fill fine grid ghost by interpolating from coarse)
    // unlike FillPatch, FillIntermediatePatch will use the supplied multifabs instead of fine level data.
    // This is to support filling boundary cells at an intermediate time between old/new times
    // on the fine level when valid data at a specific time is already available (such as
    // at each RK stage when integrating between initial and final times at a given level).
    // NOTE: mfs should always contain {cons, xvel, yvel, zvel} multifab data.
    // if which_var is supplied, then only fill the specified variable in the vector of mfs
    void FillIntermediatePatch (int lev, amrex::Real time,
                                const amrex::Vector<amrex::MultiFab*>& mfs,
                                int ng_cons, int ng_vel, bool cons_only, int icomp_cons, int ncomp_cons,
                                amrex::MultiFab* eddyDiffs, bool allow_most_bcs = true);

    // Fill all multifabs (and all components) in a vector of multifabs corresponding to the
    // grid variables defined in vars_old and vars_new just as FillCoarsePatch.
    void FillCoarsePatch (int lev, amrex::Real time, const amrex::Vector<amrex::MultiFab*>& mfs);

    // advance a level by dt
    // includes a recursive call for finer levels
    void timeStep (int lev, amrex::Real time, int iteration);

#if defined(ERF_USE_WARM_NO_PRECIP)
    void condensation_source (amrex::MultiFab& source, amrex::MultiFab& S_new,
                              amrex::Real tau_cond, amrex::Real c_p);
#endif

    // advance a single level for a single time step
    void Advance (int lev, amrex::Real time, amrex::Real dt_lev, int iteration, int ncycle);

    //! Initialize HSE
    void initHSE ();

    //! Initialize Rayleigh damping profiles
    void initRayleigh ();

    //! Set Rayleigh mean profiles from input sounding
    void setRayleighRefFromSounding (bool restarting);

    // a wrapper for estTimeStep()
    void ComputeDt ();

    // get plotfile name
    [[nodiscard]] std::string PlotFileName (int lev) const;

    // set plotfile variables names
    static amrex::Vector<std::string> PlotFileVarNames (amrex::Vector<std::string> plot_var_names) ;

    // set which variables and derived quantities go into plotfiles
    void setPlotVariables (const std::string& pp_plot_var_names, amrex::Vector<std::string>& plot_var_names);

#ifdef ERF_USE_NETCDF
    //! Write plotfile using NETCDF
    void writeNCPlotFile(int lev, int which, const std::string& dir,
                         const amrex::Vector<const amrex::MultiFab*> &mf,
                         const amrex::Vector<std::string> &varnames,
                         const amrex::Vector<int> level_steps, const amrex::Real time) const;

    //! Write checkpointFile using NetCdf
    void WriteNCCheckpointFile () const;

    //! Read checkpointFile for restart
    void ReadNCCheckpointFile ();

    //! Write MultiFab in NetCDF format
    void WriteNCMultiFab (const amrex::FabArray<amrex::FArrayBox> &fab,
                          const std::string& name,
                          bool set_ghost = false) const;

    //! Read MultiFab in NetCDF format
    void ReadNCMultiFab (amrex::FabArray<amrex::FArrayBox> &fab,
                         const std::string &name,
                         int coordinatorProc = amrex::ParallelDescriptor::IOProcessorNumber(),
                         int allow_empty_mf = 0);

    //! Create 1D vertical column output for coupling
    void createNCColumnFile(const int lev,
                            const std::string& colfile_name, const amrex::Real xloc, const amrex::Real yloc);

    // Copy from the NC*fabs into the MultiFabs holding the boundary data
    void init_from_wrfbdy(amrex::Vector<amrex::FArrayBox*> x_vel_lateral,
                          amrex::Vector<amrex::FArrayBox*> y_vel_lateral,
                          amrex::Vector<amrex::FArrayBox*> z_vel_lateral,
                          amrex::Vector<amrex::FArrayBox*> T_lateral);

    // *** *** FArrayBox's for holding the SURFACE data
    // amrex::IArrayBox NC_IVGTYP_fab; // Vegetation type (IVGTYP); Discrete numbers;
    // amrex::FArrayBox NC_z0_fab;     // Surface Roughness, z0 = z0 (IVGTYP)
    // amrex::FArrayBox NC_PSFC_fab;   // Surface pressure

    // TODO: Clarify the relation between SST and TSK
    // amrex::FArrayBox NC_SST_fab;    // Sea Surface Temperature; Defined even for land area
    // amrex::FArrayBox NC_TSK_fab;    // Surface Skin Temperature; Appears to be same as SST...

    // Vectors (over time) of Vector (over variables) of FArrayBoxs for holding the data read from the wrfbdy NetCDF file
    amrex::Vector<amrex::Vector<amrex::FArrayBox>> bdy_data_xlo;
    amrex::Vector<amrex::Vector<amrex::FArrayBox>> bdy_data_xhi;
    amrex::Vector<amrex::Vector<amrex::FArrayBox>> bdy_data_ylo;
    amrex::Vector<amrex::Vector<amrex::FArrayBox>> bdy_data_yhi;

    amrex::Real bdy_time_interval;
#endif // ERF_USE_NETCDF

    // Struct for working with the sounding data we take as an input
    InputSoundingData input_sounding_data;

    // write checkpoint file to disk
    void WriteCheckpointFile () const;

    // read checkpoint file from disk
    void ReadCheckpointFile ();

    // Read the file passed to amr.restart and use it as an initial condition for
    // the current simulation. Supports a different number of components and
    // ghost cells.
    void InitializeFromFile ();

    // Initialize the new-time data at a level from the initial_data MultiFab
    void InitializeLevelFromData (int lev, const amrex::MultiFab& initial_data);

    // utility to skip to next line in Header
    static void GotoNextLine (std::istream& is);

    // Single level functions called by advance()
    void post_update (amrex::MultiFab& state_mf, amrex::Real time, const amrex::Geometry& geom);
    void fill_rhs (amrex::MultiFab& rhs_mf, const amrex::MultiFab& state_mf, amrex::Real time, const amrex::Geometry& geom);

    ////////////////
    // private data members

    amrex::Vector<int> num_boxes_at_level;                   // how many boxes specified at each level by tagging criteria
    amrex::Vector<int> num_files_at_level;                   // how many wrfinput files specified at each level
    amrex::Vector<amrex::Vector<amrex::Box>> boxes_at_level; //      the boxes specified at each level by tagging criteria

    amrex::Vector<int> istep;      // which step?
    amrex::Vector<int> nsubsteps;  // how many substeps on each level?

    // keep track of old time, new time, and time step at each level
    amrex::Vector<amrex::Real> t_new;
    amrex::Vector<amrex::Real> t_old;
    amrex::Vector<amrex::Real> dt;
    amrex::Vector<long> dt_mri_ratio;

    // array of multifabs to store the solution at each level of refinement
    // after advancing a level we use "swap".
#ifndef ERF_USE_MULTIBLOCK
    amrex::Vector<amrex::Vector<amrex::MultiFab> > vars_new;
    amrex::Vector<amrex::Vector<amrex::MultiFab> > vars_old;
#endif
    amrex::Vector<std::unique_ptr<MRISplitIntegrator<amrex::Vector<amrex::MultiFab> > > > mri_integrator_mem;
    amrex::Vector<std::unique_ptr<ERFPhysBCFunct>> physbcs;

    // BoxArray at each level to define where we actually evolve the solution
    amrex::Vector<amrex::BoxArray> grids_to_evolve;

    // Store Theta variable for MOST BC
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> Theta_prim;

    // Scratch space for time integrator
    amrex::Vector<amrex::MultiFab> rU_old;
    amrex::Vector<amrex::MultiFab> rU_new;
    amrex::Vector<amrex::MultiFab> rV_old;
    amrex::Vector<amrex::MultiFab> rV_new;
    amrex::Vector<amrex::MultiFab> rW_old;
    amrex::Vector<amrex::MultiFab> rW_new;

#if defined(ERF_USE_MOISTURE)
    Microphysics micro;
    amrex::Vector<amrex::MultiFab> qv;
    amrex::Vector<amrex::MultiFab> qc;
    amrex::Vector<amrex::MultiFab> qi;
    amrex::Vector<amrex::MultiFab> qrain;
    amrex::Vector<amrex::MultiFab> qsnow;
    amrex::Vector<amrex::MultiFab> qgraup;
#elif defined(ERF_USE_WARM_NO_PRECIP)
#endif

    // Diffusive stresses and Smag
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> Tau11_lev, Tau22_lev, Tau33_lev;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> Tau12_lev, Tau21_lev;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> Tau13_lev, Tau31_lev;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> Tau23_lev, Tau32_lev;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> eddyDiffs_lev;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> SmnSmn_lev;

    // Other SFS terms
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> SFS_hfx1_lev, SFS_hfx2_lev, SFS_hfx3_lev;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> SFS_diss_lev;

    amrex::Vector<std::unique_ptr<amrex::MultiFab>> z_phys_nd;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> z_phys_cc;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>>   detJ_cc;

    amrex::Vector<std::unique_ptr<amrex::MultiFab>> z_phys_nd_src;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>>   detJ_cc_src;

    amrex::Vector<std::unique_ptr<amrex::MultiFab>> z_phys_nd_new;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>>   detJ_cc_new;

    amrex::Vector<std::unique_ptr<amrex::MultiFab>> z_t_rk;

    amrex::Vector<std::unique_ptr<amrex::MultiFab>> mapfac_m;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> mapfac_u;
    amrex::Vector<std::unique_ptr<amrex::MultiFab>> mapfac_v;

    amrex::Vector<std::unique_ptr<amrex::MultiFab>> sst;

    amrex::Vector<amrex::MultiFab> base_state;
    amrex::Vector<amrex::MultiFab> base_state_new;

    // array of flux registers
    amrex::Vector<amrex::FluxRegister*> flux_registers;

    // A BCRec is essentially a 2*DIM integer array storing the boundary
    // condition type at each lo/hi walls in each direction. We have one BCRec
    // for each component of the cell-centered variables and each velocity component.
    amrex::Vector           <amrex::BCRec> domain_bcs_type;
    amrex::Gpu::DeviceVector<amrex::BCRec> domain_bcs_type_d;

    // We store these so that we can print them out in the job_info file
    amrex::Array<std::string,2*AMREX_SPACEDIM> domain_bc_type;

    // These hold the Dirichlet values at walls which need them ...
    amrex::Array<amrex::Array<amrex::Real, AMREX_SPACEDIM*2>,AMREX_SPACEDIM+NVAR> m_bc_extdir_vals;

    // These hold the Neumann values at walls which need them ...
    amrex::Array<amrex::Array<amrex::Real, AMREX_SPACEDIM*2>,AMREX_SPACEDIM+NVAR> m_bc_neumann_vals;

    // These are the "physical" boundary condition types (e.g. "inflow")
    amrex::GpuArray<ERF_BC, AMREX_SPACEDIM*2> phys_bc_type;

    int last_plot_file_step_1;
    int last_plot_file_step_2;

    int last_check_file_step;
    int plot_file_on_restart = 1;

    ////////////////
    // runtime parameters

    // maximum number of steps and stop time
    int max_step = std::numeric_limits<int>::max();
    amrex::Real stop_time = std::numeric_limits<amrex::Real>::max();

    // if >= 0 we restart from a checkpoint
    std::string restart_chkfile = "";

    // Time step controls
    static amrex::Real cfl;
    static amrex::Real init_shrink;
    static amrex::Real change_max;

    // Fixed dt for level 0 timesteps (only used if positive)
    static amrex::Real fixed_dt;
    static amrex::Real fixed_fast_dt;
    static int fixed_mri_dt_ratio;

    // how often each level regrids the higher levels of refinement
    // (after a level advances that many time steps)
    int regrid_int = 2;

    // plotfile prefix and frequency
    std::string plot_file_1 {"plt_1_"};
    std::string plot_file_2 {"plt_2_"};
    int plot_int_1 = -1;
    int plot_int_2 = -1;

    // other sampling output control
    int profile_int = -1;

    // Checkpoint type, prefix and frequency
    std::string check_file {"chk"};
    std::string check_type {"native"};
    std::string restart_type {"native"};
    int check_int = -1;

    amrex::Vector<std::string> plot_var_names_1;
    amrex::Vector<std::string> plot_var_names_2;
    const amrex::Vector<std::string> velocity_names {"x_velocity", "y_velocity", "z_velocity"};
    const amrex::Vector<std::string> cons_names     {"density", "rhotheta", "rhoKE", "rhoQKE", "rhoadv_0"
#if defined(ERF_USE_MOISTURE)
                                                     ,"rhoQt", "rhoQp"
#elif defined(ERF_USE_WARM_NO_PRECIP)
                                                     ,"rhoQv", "rhoQc"
#endif
                                                     };

    // Note that the order of variable names here must match the order in Derive.cpp
    const amrex::Vector<std::string> derived_names {"pressure", "soundspeed", "temp", "theta", "KE", "QKE", "scalar",
                                                    "pres_hse", "dens_hse", "pert_pres", "pert_dens",
                                                    "dpdx", "dpdy", "pres_hse_x", "pres_hse_y", "z_phys", "detJ" , "mapfac"
#if defined(ERF_USE_MOISTURE)
                                                     ,"qt", "qp", "qv", "qc", "qi", "qrain", "qsnow", "qgraup"
#elif defined(ERF_USE_WARM_NO_PRECIP)
                                                     ,"qv", "qc"
#endif
#ifdef ERF_COMPUTE_ERROR
                                                    ,"xvel_err", "yvel_err", "zvel_err", "pp_err"
#endif
                                                   };

    // algorithm choices
    static SolverChoice solverChoice;

    static int verbose;
    static int use_native_mri;
    static int no_substepping;
    static int force_stage1_single_substep;

    // mesh refinement
    static std::string coupling_type;
    static int do_reflux;
    static int do_avg_down;

    // Diagnostic output interval
    static int sum_interval;
    static amrex::Real sum_per;

    // Native or NetCDF
    static std::string plotfile_type;

    // init_type:  "ideal", "real", "input_sounding", "metgrid" or ""
    static std::string init_type;

    // NetCDF initialization (wrfinput) file
    static amrex::Vector<amrex::Vector<std::string>> nc_init_file;

    // NetCDF initialization (wrfbdy) file
    static std::string nc_bdy_file;
    int wrfbdy_width;

    // Text input_sounding file
    static std::string input_sounding_file;

    // Flag to trigger initialization from input_sounding like WRF's ideal.exe
    // used with init_type == "input_sounding"
    static bool init_sounding_ideal;

    // 1D CDF output (for ingestion in AMR-Wind)
    static int         output_1d_column;
    static int         column_interval;
    static amrex::Real column_per;
    static amrex::Real column_loc_x;
    static amrex::Real column_loc_y;
    static std::string column_file_name;

    // 2D BndryRegister output (for ingestion in AMR-Wind)
    static int         output_bndry_planes;
    static int         bndry_output_planes_interval;
    static amrex::Real bndry_output_planes_per;
    static amrex::Real bndry_output_planes_start_time;

    // 2D BndryRegister input
    static int          input_bndry_planes;

    static int ng_dens_hse;
    static int ng_pres_hse;

    // Mean quantities and tau for Rayleigh damping
    amrex::Vector<amrex::Vector<amrex::Real> > h_rayleigh_tau;
    amrex::Vector<amrex::Vector<amrex::Real> > h_rayleigh_ubar;
    amrex::Vector<amrex::Vector<amrex::Real> > h_rayleigh_vbar;
    amrex::Vector<amrex::Vector<amrex::Real> > h_rayleigh_wbar;
    amrex::Vector<amrex::Vector<amrex::Real> > h_rayleigh_thetabar;
    amrex::Vector<amrex::Gpu::DeviceVector<amrex::Real> > d_rayleigh_tau;
    amrex::Vector<amrex::Gpu::DeviceVector<amrex::Real> > d_rayleigh_ubar;
    amrex::Vector<amrex::Gpu::DeviceVector<amrex::Real> > d_rayleigh_vbar;
    amrex::Vector<amrex::Gpu::DeviceVector<amrex::Real> > d_rayleigh_wbar;
    amrex::Vector<amrex::Gpu::DeviceVector<amrex::Real> > d_rayleigh_thetabar;

    amrex::Vector<amrex::Real> h_havg_density;
    amrex::Vector<amrex::Real> h_havg_temperature;
    amrex::Vector<amrex::Real> h_havg_pressure;
#ifdef ERF_USE_MOISTURE
    amrex::Vector<amrex::Real> h_havg_qv;
    amrex::Vector<amrex::Real> h_havg_qc;
#endif
    amrex::Gpu::DeviceVector<amrex::Real> d_havg_density;
    amrex::Gpu::DeviceVector<amrex::Real> d_havg_temperature;
    amrex::Gpu::DeviceVector<amrex::Real> d_havg_pressure;
#ifdef ERF_USE_MOISTURE
    amrex::Gpu::DeviceVector<amrex::Real> d_havg_qv;
    amrex::Gpu::DeviceVector<amrex::Real> d_havg_qc;
#endif

    void refinement_criteria_setup();

    std::unique_ptr<WriteBndryPlanes> m_w2d  = nullptr;
    std::unique_ptr<ReadBndryPlanes>  m_r2d  = nullptr;
    std::unique_ptr<ABLMost>          m_most = nullptr;

    //
    // Holds info for dynamically generated tagging criteria
    //
    static amrex::Vector<amrex::AMRErrorTag> ref_tags;

    //
    // Build a mask that zeroes out values on a coarse level underlying
    //     grids on the next finest level
    //
    amrex::MultiFab fine_mask;

    static AMREX_FORCE_INLINE
    int
    ComputeGhostCells(const int& horiz_spatial_order, const int& vert_spatial_order,
                      const int& spatial_order_WENO,  const bool& use_WENO,
                      const bool& use_numerical_diff) {
      int nGhostCells;

      int spatial_order = std::max(horiz_spatial_order, vert_spatial_order);

      if (use_numerical_diff) {
          nGhostCells = 3;
      } else if(use_WENO) {
          switch (spatial_order_WENO) {
          case 3:
              nGhostCells = 2;
              break;
          case 5:
              nGhostCells = 3;
              break;
          default:
              amrex::Error("Must specify WENO spatial order to be 3 or 5");
              break;
          }
      } else {
          switch (spatial_order) {
          case 2: case 3: case 4:
              nGhostCells = 2; // We need this many to compute the eddy viscosity in the ghost cells
              break;
          case 5: case 6:
              nGhostCells = 3;
              break;
          default:
              amrex::Error("Must specify spatial order to be 2,3,4,5 or 6");
          }
      }

      return nGhostCells;
    }

    AMREX_FORCE_INLINE
    amrex::FluxRegister&
    get_flux_reg(int lev)
    {
      return *flux_registers[lev];
    }

    AMREX_FORCE_INLINE
    std::ostream&
    DataLog (int i)
    {
        return *datalog[i];
    }

    AMREX_FORCE_INLINE
    int
    NumDataLogs () noexcept
    {
        return datalog.size();
    }

    AMREX_FORCE_INLINE
    std::ostream&
    SamplePointLog (int i)
    {
        return *sampleptlog[i];
    }

    AMREX_FORCE_INLINE
    int
    NumSamplePointLogs () noexcept
    {
        return sampleptlog.size();
    }

    AMREX_FORCE_INLINE
    std::ostream&
    SampleLineLog (int i)
    {
        return *samplelinelog[i];
    }

    AMREX_FORCE_INLINE
    int
    NumSampleLineLogs () noexcept
    {
        return samplelinelog.size();
    }

    amrex::IntVect&
    SamplePoint (int i)
    {
        return samplepoint[i];
    }

    AMREX_FORCE_INLINE
    int
    NumSamplePoints () noexcept
    {
        return samplepoint.size();
    }

    amrex::IntVect&
    SampleLine (int i)
    {
        return sampleline[i];
    }

    AMREX_FORCE_INLINE
    int
    NumSampleLines () noexcept
    {
        return sampleline.size();
    }

    static amrex::Real startCPUTime;
    static amrex::Real previousCPUTimeUsed;

    static amrex::Real
    getCPUTime()
    {
      int numCores = amrex::ParallelDescriptor::NProcs();
#ifdef _OPENMP
      numCores = numCores * omp_get_max_threads();
#endif

      amrex::Real T =
        numCores * (amrex::ParallelDescriptor::second() - startCPUTime) +
        previousCPUTimeUsed;

      return T;
    }

    void setRecordDataInfo (int i, const std::string& filename) // NOLINT
    {
        if (amrex::ParallelDescriptor::IOProcessor())
        {
            datalog[i] = std::make_unique<std::fstream>();
            datalog[i]->open(filename.c_str(),std::ios::out|std::ios::app);
            if (!datalog[i]->good()) {
                amrex::FileOpenFailed(filename);
            }
        }
        amrex::ParallelDescriptor::Barrier("ERF::setRecordDataInfo");
    }

    void setRecordSamplePointInfo (int i, int lev, amrex::IntVect& cell, const std::string& filename) // NOLINT
    {
        amrex::MultiFab dummy(grids[lev],dmap[lev],1,0);
        for (amrex::MFIter mfi(dummy); mfi.isValid(); ++mfi)
        {
            const amrex::Box& bx = mfi.validbox();
            if (bx.contains(cell)) {
                sampleptlog[i] = std::make_unique<std::fstream>();
                sampleptlog[i]->open(filename.c_str(),std::ios::out|std::ios::app);
                if (!sampleptlog[i]->good()) {
                    amrex::FileOpenFailed(filename);
                }
            }
        }
        amrex::ParallelDescriptor::Barrier("ERF::setRecordSamplePointInfo");
    }

    void setRecordSampleLineInfo (int i, int lev, amrex::IntVect& cell, const std::string& filename) // NOLINT
    {
        amrex::MultiFab dummy(grids[lev],dmap[lev],1,0);
        for (amrex::MFIter mfi(dummy); mfi.isValid(); ++mfi)
        {
            const amrex::Box& bx = mfi.validbox();
            if (bx.contains(cell)) {
                samplelinelog[i] = std::make_unique<std::fstream>();
                samplelinelog[i]->open(filename.c_str(),std::ios::out|std::ios::app);
                if (!samplelinelog[i]->good()) {
                    amrex::FileOpenFailed(filename);
                }
            }
        }
        amrex::ParallelDescriptor::Barrier("ERF::setRecordSampleLineInfo");
    }

    amrex::Vector<std::unique_ptr<std::fstream> > datalog;
    amrex::Vector<std::string> datalogname;

    amrex::Vector<std::unique_ptr<std::fstream> > sampleptlog;
    amrex::Vector<std::string> sampleptlogname;
    amrex::Vector<amrex::IntVect> samplepoint;

    amrex::Vector<std::unique_ptr<std::fstream> > samplelinelog;
    amrex::Vector<std::string> samplelinelogname;
    amrex::Vector<amrex::IntVect> sampleline;

    //! The filename of the ith datalog file.
    [[nodiscard]] std::string DataLogName (int i) const noexcept { return datalogname[i]; }

    //! The filename of the ith sampleptlog file.
    [[nodiscard]] std::string SamplePointLogName (int i) const noexcept { return sampleptlogname[i]; }

    //! The filename of the ith samplelinelog file.
    [[nodiscard]] std::string SampleLineLogName (int i) const noexcept { return samplelinelogname[i]; }

public:
    void writeJobInfo(const std::string& dir) const;
    static void writeBuildInfo(std::ostream& os);
};

#endif

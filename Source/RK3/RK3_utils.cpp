// Generated by Pankaj Jha on 05/05/2021
#include <RK3.H>

using namespace amrex;

int
ComputeGhostCells(const int& spatial_order) {
  int nGhostCells;

  //TODO: Make sure we have correct number of ghost cells for different spatial orders.
  switch (spatial_order) {
    case 1:
      nGhostCells = 1;
      break;
    case 2:
      nGhostCells = 1;
      break;
    case 3:
      nGhostCells = 1;
      break;
    case 4:
      nGhostCells = 1;
      break;
    case 5:
      nGhostCells = 1;
      break;
    case 6:
      nGhostCells = 1;
      break;
    default:
      nGhostCells = 1;
  }

  return nGhostCells;
}

Real
InterpolateDensityFromCellToFace(
  const int& i,
  const int& j,
  const int& k,
  const Array4<Real>& cons_in,
  const NextOrPrev& nextOrPrev,
  const AdvectionDir& advectionDir,
  const int& spatial_order)
{
  return InterpolateFromCellToFace(i, j, k, cons_in, Density_comp,
                                     nextOrPrev, advectionDir, spatial_order);
}

Real
InterpolateRhoThetaFromCellToFace(
  const int& i,
  const int& j,
  const int& k,
  const Array4<Real>& cons_in,
  const NextOrPrev& nextOrPrev,
  const AdvectionDir& advectionDir,
  const int& spatial_order)
{
  return InterpolateFromCellToFace(i, j, k, cons_in, RhoTheta_comp,
                                   nextOrPrev, advectionDir, spatial_order);
}
Real
InterpolateScalarFromCellToFace(
  const int& i,
  const int& j,
  const int& k,
  const Array4<Real>& cons_in,
  const NextOrPrev& nextOrPrev,
  const AdvectionDir& advectionDir,
  const int& spatial_order)
{
  return InterpolateFromCellToFace(i, j, k, cons_in, Scalar_comp,
                                   nextOrPrev, advectionDir, spatial_order);
}

Real
InterpolateFromCellToFace(
  // (i, j, k) is the reference cell index w.r.t. which a face is being considered
  const int& i, const int& j, const int& k,
  const Array4<Real>& cons_in,
  const int& cons_qty_index,
  const NextOrPrev& nextOrPrev,
  const AdvectionDir& advectionDir,
  const int& spatial_order)
{
  /*
   If the interpolation is to be done on face (i, j, k) which is previous to cell (i,j,k)
   in the coordinate direction,
   call as InterpolateFromCellToFace(i, j, k, cons_in, cons_qty_index, NextOrPrev::prev, ...)
   */
  /*
   If the interpolation is to be done on face (i+1, j, k), (i, j+1, k) or (i, j, k+1)
   which are next to cell (i,j,k) in the coordinate direction,
   call as InterpolateFromCellToFace(i, j, k, cons_in, cons_qty_index, NextOrPrev::next, ...)
   */

  Real interpolatedVal = 0.0;
  if (nextOrPrev == NextOrPrev::prev) {
    /*
     w.r.t. the cell index (i, j, k), the face previous to it is the face at cell index m-1/2, where m = {i, j, k}
     Face index is (i, j, k). This means:
     Coordinates of face (i, j, k) = Coordinates of cell (i-1/2, j    , k    ) for x-dir. Face is previous to the cell.
     Coordinates of face (i, j, k) = Coordinates of cell (i    , j-1/2, k    ) for y-dir. Face is previous to the cell.
     Coordinates of face (i, j, k) = Coordinates of cell (i    , j    , k-1/2) for z-dir. Face is previous to the cell.
    */
    switch (spatial_order) {
      case 2:
        switch (advectionDir) {
          // q = cons_in(i, j, k, cons_qty_index) = {rho, theta, rhoTheta, scalar, pressure, ...}
          case AdvectionDir::x: // m = i, q(m-1/2) = q(i-1/2, j    , k    )
            interpolatedVal = 0.5*(cons_in(i, j, k, cons_qty_index) + cons_in(i-1, j, k, cons_qty_index));
            break;
          case AdvectionDir::y: // m = j, q(m-1/2) = q(i    , j-1/2, k    )
            interpolatedVal = 0.5*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j-1, k, cons_qty_index));
            break;
          case AdvectionDir::z: // m = k, q(m-1/2) = q(i    , j    , k-1/2)
            interpolatedVal = 0.5*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j, k-1, cons_qty_index));
            break;
          default:
            amrex::Abort("Error: Advection direction is unrecognized");
        }
        break;
      case 4:
        switch (advectionDir) {
          // q = cons_in(i, j, k, cons_qty_index) = {rho, theta, rhoTheta, scalar, pressure, ...}
          case AdvectionDir::x: // m = i, q(m-1/2) = q(i-1/2, j    , k    )
            interpolatedVal = (7.0/12.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i-1, j, k, cons_qty_index))
                             -(1.0/12.0)*(cons_in(i+1, j, k, cons_qty_index) + cons_in(i-2, j, k, cons_qty_index));
            break;
          case AdvectionDir::y: // m = j, q(m-1/2) = q(i    , j-1/2, k    )
            interpolatedVal = (7.0/12.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j-1, k, cons_qty_index))
                             -(1.0/12.0)*(cons_in(i, j+1, k, cons_qty_index) + cons_in(i, j-2, k, cons_qty_index));
            break;
          case AdvectionDir::z: // m = k, q(m-1/2) = q(i    , j    , k-1/2)
            interpolatedVal = (7.0/12.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j, k-1, cons_qty_index))
                             -(1.0/12.0)*(cons_in(i, j, k+1, cons_qty_index) + cons_in(i, j, k-2, cons_qty_index));
            break;
          default:
            amrex::Abort("Error: Advection direction is unrecognized");
        }
        break;
      case 6: // In order to make this work 'cons_in' must have indices m-3 and m+2 where m = {i, j, k}
        switch (advectionDir) {
          // q = cons_in(i, j, k, cons_qty_index) = {rho, theta, rhoTheta, scalar, pressure, ...}
          case AdvectionDir::x: // m = i, q(m-1/2) = q(i-1/2, j    , k    )
            interpolatedVal = (37.0/60.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i-1, j, k, cons_qty_index))
                              -(2.0/15.0)*(cons_in(i+1, j, k, cons_qty_index) + cons_in(i-2, j, k, cons_qty_index))
                              +(1.0/60.0)*(cons_in(i+2, j, k, cons_qty_index) + cons_in(i-3, j, k, cons_qty_index));
            break;
          case AdvectionDir::y: // m = j, q(m-1/2) = q(i    , j-1/2, k    )
            interpolatedVal = (37.0/60.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j-1, k, cons_qty_index))
                              -(2.0/15.0)*(cons_in(i, j+1, k, cons_qty_index) + cons_in(i, j-2, k, cons_qty_index))
                              +(1.0/60.0)*(cons_in(i, j+2, k, cons_qty_index) + cons_in(i, j-3, k, cons_qty_index));
            break;
          case AdvectionDir::z: // m = k, q(m-1/2) = q(i    , j    , k-1/2)
            interpolatedVal = (37.0/60.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j, k-1, cons_qty_index))
                              -(2.0/15.0)*(cons_in(i, j, k+1, cons_qty_index) + cons_in(i, j, k-2, cons_qty_index))
                              +(1.0/60.0)*(cons_in(i, j, k+2, cons_qty_index) + cons_in(i, j, k-3, cons_qty_index));
            break;
          default:
            amrex::Abort("Error: Advection direction is unrecognized");
        }
        break;
      default:
        amrex::Abort("Error: Spatial order " + std::to_string(spatial_order) + " has not been implemented");
    }
  }
  else { // nextOrPrev == NextOrPrev::next
    /*
     w.r.t. the cell index (i, j, k), the face next to it is the face at cell index m+1/2, where m = {i, j, k}
     This means:
     Coordinates of face (i+1, j  , k  ) = Coordinates of cell (i+1/2, j    , k    ) for x-dir. Face is next to the cell.
     Coordinates of face (i  , j+1, k  ) = Coordinates of cell (i    , j+1/2, k    ) for y-dir. Face is next to the cell.
     Coordinates of face (i  , j  , k+1) = Coordinates of cell (i    , j    , k+1/2) for z-dir. Face is next to the cell.
    */
    switch (spatial_order) {
      case 2:
        switch (advectionDir) {
          // q = cons_in(i, j, k, cons_qty_index) = {rho, theta, rhoTheta, scalar, pressure, ...}
          case AdvectionDir::x: // m = i, q(m+1/2) = q(i+1/2, j    , k    )
            interpolatedVal = 0.5*(cons_in(i, j, k, cons_qty_index) + cons_in(i+1, j, k, cons_qty_index));
            break;
          case AdvectionDir::y: // m = j, q(m+1/2) = q(i    , j+1/2, k    )
            interpolatedVal = 0.5*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j+1, k, cons_qty_index));
            break;
          case AdvectionDir::z: // m = k, q(m+1/2) = q(i    , j    , k+1/2)
            interpolatedVal = 0.5*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j, k+1, cons_qty_index));
            break;
          default:
            amrex::Abort("Error: Advection direction is unrecognized");
        }
        break;
      case 4:
        switch (advectionDir) {
          // q = cons_in(i, j, k, cons_qty_index) = {rho, theta, rhoTheta, scalar, pressure, ...}
          case AdvectionDir::x: // m = i, q(m+1/2) = q(i+1/2, j    , k    )
            interpolatedVal = (7.0/12.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i+1, j, k, cons_qty_index))
                              -(1.0/12.0)*(cons_in(i-1, j, k, cons_qty_index) + cons_in(i+2, j, k, cons_qty_index));
            break;
          case AdvectionDir::y: // m = j, q(m+1/2) = q(i    , j+1/2, k    )
            interpolatedVal = (7.0/12.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j+1, k, cons_qty_index))
                              -(1.0/12.0)*(cons_in(i, j-1, k, cons_qty_index) + cons_in(i, j+2, k, cons_qty_index));
            break;
          case AdvectionDir::z: // m = k, q(m+1/2) = q(i    , j    , k+1/2)
            interpolatedVal = (7.0/12.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j, k+1, cons_qty_index))
                              -(1.0/12.0)*(cons_in(i, j, k-1, cons_qty_index) + cons_in(i, j, k+2, cons_qty_index));
            break;
          default:
            amrex::Abort("Error: Advection direction is unrecognized");
        }
        break;
      case 6: // In order to make this work 'cons_in' must have indices m+3 and m-2 where m = {i, j, k}
        switch (advectionDir) {
          // q = cons_in(i, j, k, cons_qty_index) = {rho, theta, rhoTheta, scalar, pressure, ...}
          case AdvectionDir::x: // m = i, q(m+1/2) = q(i+1/2, j    , k    )
            interpolatedVal = (37.0/60.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i+1, j, k, cons_qty_index))
                              -(2.0/15.0)*(cons_in(i-1, j, k, cons_qty_index) + cons_in(i+2, j, k, cons_qty_index))
                              +(1.0/60.0)*(cons_in(i-2, j, k, cons_qty_index) + cons_in(i+3, j, k, cons_qty_index));
            break;
          case AdvectionDir::y: // m = j, q(m+1/2) = q(i    , j+1/2, k    )
            interpolatedVal = (37.0/60.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j+1, k, cons_qty_index))
                              -(2.0/15.0)*(cons_in(i, j-1, k, cons_qty_index) + cons_in(i, j+2, k, cons_qty_index))
                              +(1.0/60.0)*(cons_in(i, j-2, k, cons_qty_index) + cons_in(i, j+3, k, cons_qty_index));
            break;
          case AdvectionDir::z: // m = k, q(m+1/2) = q(i    , j    , k+1/2)
            interpolatedVal = (37.0/60.0)*(cons_in(i, j, k, cons_qty_index) + cons_in(i, j, k+1, cons_qty_index))
                              -(2.0/15.0)*(cons_in(i, j, k-1, cons_qty_index) + cons_in(i, j, k+2, cons_qty_index))
                              +(1.0/60.0)*(cons_in(i, j, k-2, cons_qty_index) + cons_in(i, j, k+3, cons_qty_index));
            break;
          default:
            amrex::Abort("Error: Advection direction is unrecognized");
        }
        break;
      default:
        amrex::Abort("Error: Spatial order " + std::to_string(spatial_order) + " has not been implemented");
    }
  }

  return interpolatedVal;
}

Real
ComputeAdvectedQuantityForMom(const int &i, const int &j, const int &k,
                             const Array4<Real>& rho_u, const Array4<Real>& rho_v, const Array4<Real>& rho_w,
                             const Array4<Real>& u, const Array4<Real>& v, const Array4<Real>& w,
                             const enum NextOrPrev &nextOrPrev,
                             const enum AdvectedQuantity &advectedQuantity,
                             const enum AdvectingQuantity &advectingQuantity,
                             const int &spatial_order) {
  Real advectingQty = 0.0;
  Real advectedQty = 1.0;
  if (nextOrPrev == NextOrPrev::next) {
    switch(advectedQuantity) {
      case AdvectedQuantity::u: //x-momentum, reference face index is (i, j, k)
        switch (advectingQuantity) {
          case AdvectingQuantity::rho_u:
            advectedQty = 1.0; // u(i+1/2,    j, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_u(i+1, j, k) + rho_u(i, j, k)); // Effectively rho_u (i+1/2, j, k)
            break;
          case AdvectingQuantity::rho_v:
            advectedQty = 1.0; // u(i   , j+1/2, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_v(i, j+1, k) + rho_v(i-1, j+1, k)); // Effectively rho_v (i-1/2, j+1, k)
            break;
          case AdvectingQuantity::rho_w:
            advectedQty = 1.0; // u(i   , j    , k+1/2)..Needs to be called properly
            advectingQty = 0.5*(rho_w(i, j, k+1) + rho_w(i-1, j, k+1)); // Effectively rho_w (i-1/2, j, k+1)
            break;
          default:
            amrex::Abort("Error: Advecting quantity is unrecognized");
        }
        break;
      case AdvectedQuantity::v: //y-momentum, reference face index is (i, j, k)
        switch (advectingQuantity) {
          case AdvectingQuantity::rho_u:
            advectedQty = 1.0; // v(i+1/2,    j, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_u(i+1, j, k) + rho_u(i+1, j-1, k)); // Effectively rho_u (i+1, j-1/2, k)
            break;
          case AdvectingQuantity::rho_v:
            advectedQty = 1.0; // v(i   , j+1/2, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_v(i, j+1, k) + rho_v(i, j, k)); // Effectively rho_v (i, j+1/2, k)
            break;
          case AdvectingQuantity::rho_w:
            advectedQty = 1.0; // v(i   , j    , k+1/2)..Needs to be called properly
            advectingQty = 0.5*(rho_w(i, j, k+1) + rho_w(i, j-1, k+1)); // Effectively rho_w (i, j-1/2, k+1)
            break;
          default:
            amrex::Abort("Error: Advecting quantity is unrecognized");
        }
        break;
      case AdvectedQuantity::w: //z-momentum, reference face index is (i, j, k)
        switch (advectingQuantity) {
          case AdvectingQuantity::rho_u:
            advectedQty = 1.0; // w(i+1/2,    j, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_u(i+1, j, k) + rho_u(i+1, j, k-1)); // Effectively rho_u (i+1, j, k-1/2)
            break;
          case AdvectingQuantity::rho_v:
            advectedQty = 1.0; // w(i   , j+1/2, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_v(i, j+1, k) + rho_v(i, j+1, k-1)); // Effectively rho_v (i, j+1, k-1/2)
            break;
          case AdvectingQuantity::rho_w:
            advectedQty = 1.0; // w(i   , j    , k+1/2)..Needs to be called properly
            advectingQty = 0.5*(rho_w(i, j, k+1) + rho_w(i, j, k)); // Effectively rho_w (i, j, k+1/2)
            break;
          default:
            amrex::Abort("Error: Advecting quantity is unrecognized");
        }
        break;
      default:
        amrex::Abort("Error: Advected quantity is unrecognized");
    }
  }
  else { // nextOrPrev == NextOrPrev::prev
    switch(advectedQuantity) {
      case AdvectedQuantity::u: //x-momentum, reference face index is (i, j, k)
        switch (advectingQuantity) {
          case AdvectingQuantity::rho_u:
            advectedQty = 1.0; // u(i-1/2,    j, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_u(i-1, j, k) + rho_u(i, j, k)); // Effectively rho_u (i-1/2, j, k)
            break;
          case AdvectingQuantity::rho_v:
            advectedQty = 1.0; // u(i   , j-1/2, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_v(i, j, k) + rho_v(i-1, j, k)); // Effectively rho_v (i-1/2, j, k)
            break;
          case AdvectingQuantity::rho_w:
            advectedQty = 1.0; // u(i   , j    , k-1/2)..Needs to be called properly
            advectingQty = 0.5*(rho_w(i, j, k) + rho_w(i-1, j, k)); // Effectively rho_w (i-1/2, j, k)
            break;
          default:
            amrex::Abort("Error: Advecting quantity is unrecognized");
        }
        break;
      case AdvectedQuantity::v: //y-momentum, reference face index is (i, j, k)
        switch (advectingQuantity) {
          case AdvectingQuantity::rho_u:
            advectedQty = 1.0; // v(i-1/2,    j, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_u(i, j, k) + rho_u(i, j-1, k)); // Effectively rho_u (i, j-1/2, k)
            break;
          case AdvectingQuantity::rho_v:
            advectedQty = 1.0; // v(i   , j-1/2, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_v(i, j, k) + rho_v(i, j-1, k)); // Effectively rho_v (i, j-1/2, k)
            break;
          case AdvectingQuantity::rho_w:
            advectedQty = 1.0; // v(i   , j    , k-1/2)..Needs to be called properly
            advectingQty = 0.5*(rho_w(i, j, k) + rho_w(i, j-1, k)); // Effectively rho_w (i, j-1/2, k)
            break;
          default:
            amrex::Abort("Error: Advecting quantity is unrecognized");
        }
        break;
      case AdvectedQuantity::w: //z-momentum, reference face index is (i, j, k)
        switch (advectingQuantity) {
          case AdvectingQuantity::rho_u:
            advectedQty = 1.0; // w(i-1/2,    j, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_u(i, j, k) + rho_u(i, j, k-1)); // Effectively rho_u (i, j, k-1/2)
            break;
          case AdvectingQuantity::rho_v:
            advectedQty = 1.0; // w(i   , j-1/2, k    )..Needs to be called properly
            advectingQty = 0.5*(rho_v(i, j, k) + rho_v(i, j, k-1)); // Effectively rho_v (i, j, k-1/2)
            break;
          case AdvectingQuantity::rho_w:
            advectedQty = 1.0; // w(i   , j    , k-1/2)..Needs to be called properly
            advectingQty = 0.5*(rho_w(i, j, k) + rho_w(i, j, k-1)); // Effectively rho_w (i, j, k-1/2)
            break;
          default:
            amrex::Abort("Error: Advecting quantity is unrecognized");
        }
        break;
      default:
        amrex::Abort("Error: Advected quantity is unrecognized");
    }
  }

  return advectingQty * advectedQty;
}

Real
ComputeAdvectedQuantityForState(const int &i, const int &j, const int &k,
                                const Array4<Real>& rho_u, const Array4<Real>& rho_v, const Array4<Real>& rho_w,
                                const Array4<Real>& cons_in,
                                const enum NextOrPrev &nextOrPrev,
                                const enum AdvectedQuantity &advectedQuantity,
                                const enum AdvectingQuantity &advectingQuantity,
                                const int &spatial_order) {
  Real advectingQty = 0.0;
  Real advectedQty = 1.0;

  /* For the all three types of state equations (continuity, energy, scalar),
   * the advecting quantities are the same */
  if (nextOrPrev == NextOrPrev::next) {
    switch (advectingQuantity) { // reference cell is (i, j, k)
      case AdvectingQuantity::rho_u:
        advectingQty = rho_u(i+1, j, k); // rho_u (i+1, j, k)
        break;
      case AdvectingQuantity::rho_v:
        advectingQty = rho_v(i, j+1, k); // rho_v (i, j+1, k)
        break;
      case AdvectingQuantity::rho_w:
        advectingQty = rho_w(i, j, k+1); // rho_w (i, j, k+1)
        break;
      default:
        amrex::Abort("Error: Advecting quantity is unrecognized");
    }
  }
  else { // nextOrPrev == NextOrPrev::prev
    switch (advectingQuantity) { // reference cell is (i, j, k)
      case AdvectingQuantity::rho_u:
        advectingQty = rho_u(i, j, k); // rho_u (i, j, k)
        break;
      case AdvectingQuantity::rho_v:
        advectingQty = rho_v(i, j, k); // rho_v (i, j, k)
        break;
      case AdvectingQuantity::rho_w:
        advectingQty = rho_w(i, j, k); // rho_w (i, j, k)
        break;
      default:
        amrex::Abort("Error: Advecting quantity is unrecognized");
    }
  }

  // Compute advected quantity for different choice of AdvectingQuantity
  switch(advectedQuantity) {
    case AdvectedQuantity::unity: // continuity equation, reference cell is (i, j, k)
      advectedQty = 1.0; // for AdvectingQuantity = {rho_u, rho_v, rho_w}
      break;
    case AdvectedQuantity::theta:
      switch (advectingQuantity) { // reference cell is (i, j, k)
        case AdvectingQuantity::rho_u:
          // Get theta (i+1/2,    j, k    ) = theta on face (i+1, j  , k  ) for x-dir if nextOrPrev = NextOrPrev::next
          // Get theta (i-1/2,    j, k    ) = theta on face (i,   j  , k  ) for x-dir if nextOrPrev = NextOrPrev::prev
          advectedQty = InterpolateRhoThetaFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::x, spatial_order);
          advectedQty/= InterpolateDensityFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::x, spatial_order);
          break;
        case AdvectingQuantity::rho_v:
          // Get theta (i   , j+1/2, k    ) = theta on face (i  , j+1, k  ) for y-dir if nextOrPrev = NextOrPrev::next
          // Get theta (i   , j-1/2, k    ) = theta on face (i  , j  , k  ) for y-dir if nextOrPrev = NextOrPrev::prev
          advectedQty = InterpolateRhoThetaFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::y, spatial_order);
          advectedQty/= InterpolateDensityFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::y, spatial_order);
          break;
        case AdvectingQuantity::rho_w:
          // Get theta (i   , j    , k+1/2) = theta on face (i  , j  , k+1) for z-dir if nextOrPrev = NextOrPrev::next
          // Get theta (i   , j    , k-1/2) = theta on face (i  , j  , k  ) for z-dir if nextOrPrev = NextOrPrev::prev
          advectedQty = InterpolateRhoThetaFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::z, spatial_order);
          advectedQty/= InterpolateDensityFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::z, spatial_order);
          break;
        default:
          amrex::Abort("Error: Advecting quantity is unrecognized");
      }
      break;
    case AdvectedQuantity::scalar:
      switch (advectingQuantity) { // reference cell is (i, j, k)
        case AdvectingQuantity::rho_u:
          // Get scalar (i+1/2,    j, k    ) = scalar on face (i+1, j  , k  ) for x-dir if nextOrPrev = NextOrPrev::next
          // Get scalar (i-1/2,    j, k    ) = scalar on face (i,   j  , k  ) for x-dir if nextOrPrev = NextOrPrev::prev
          advectedQty = InterpolateScalarFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::x, spatial_order);
          break;
        case AdvectingQuantity::rho_v:
          // Get scalar (i   , j+1/2, k    ) = scalar on face (i  , j+1, k  ) for y-dir if nextOrPrev = NextOrPrev::next
          // Get scalar (i   , j-1/2, k    ) = scalar on face (i  , j  , k  ) for y-dir if nextOrPrev = NextOrPrev::prev
          advectedQty = InterpolateScalarFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::y, spatial_order);
          break;
        case AdvectingQuantity::rho_w:
          // Get scalar (i   , j    , k+1/2) = scalar on face (i  , j  , k+1) for z-dir if nextOrPrev = NextOrPrev::next
          // Get scalar (i   , j    , k-1/2) = scalar on face (i  , j  , k  ) for z-dir if nextOrPrev = NextOrPrev::prev
          advectedQty = InterpolateScalarFromCellToFace(i, j, k, cons_in, nextOrPrev, AdvectionDir::z, spatial_order);
          break;
        default:
          amrex::Abort("Error: Advecting quantity is unrecognized");
      }
      break;
    default:
      amrex::Abort("Error: Advected quantity is unrecognized");
  }

  // Return the product of advected and advecting quantities
  return advectingQty * advectedQty;
}

Real
ComputeStressTerm (const int &i, const int &j, const int &k,
                   const Array4<Real>& u, const Array4<Real>& v, const Array4<Real>& w,
                   const enum NextOrPrev &nextOrPrev,
                   const enum MomentumEqn &momentumEqn,
                   const enum DiffusionDir &diffDir,
                   const Geometry &geom) {
  //auto cellSize = geom.CellSize();
  Real dx = geom.CellSize()[0];
  Real dy = geom.CellSize()[1];
  Real dz = geom.CellSize()[2];

  Real stressTerm = 0;

  switch (momentumEqn) {
    case MomentumEqn::x:
      switch (diffDir) {
        case DiffusionDir::x: // S11
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (u(i+1, j, k) - u(i, j, k))/dx; // S11 (i+1/2)
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (u(i, j, k) - u(i-1, j, k))/dx; // S11 (i-1/2)
          break;
        case DiffusionDir::y: // S12
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (u(i, j+1, k) - u(i, j, k))/dy + (v(i, j+1, k) - v(i-1, j+1, k))/dx; // S12 (j+1/2)
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (u(i, j, k) - u(i, j-1, k))/dy + (v(i, j, k) - v(i-1, j, k))/dx; // S12 (j-1/2)
          stressTerm*= 0.5;
          break;
        case DiffusionDir::z: // S13
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (u(i, j, k+1) - u(i, j, k))/dz + (w(i, j, k+1) - w(i-1, j, k+1))/dx; // S13 (k+1/2)
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (u(i, j, k) - u(i, j, k-1))/dz + (w(i, j, k) - w(i-1, j, k))/dx; // S13 (k-1/2)
          stressTerm*= 0.5;
          break;
        default:
          amrex::Abort("Error: Diffusion direction is unrecognized");
      }
      break;
    case MomentumEqn::y:
      switch (diffDir) {
        case DiffusionDir::x: // S21
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (u(i+1, j, k) - u(i+1, j-1, k))/dy + (v(i+1, j, k) - v(i, j, k))/dx; // S21 (i+1/2)
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (u(i, j, k) - u(i, j-1, k))/dy + (v(i, j, k) - v(i-1, j, k))/dx; // S21 (i-1/2)
          stressTerm*= 0.5;
          break;
        case DiffusionDir::y: // S22
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (v(i, j+1, k) - v(i, j, k))/dy; // S22 (j+1/2)
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (v(i, j, k) - v(i, j-1, k))/dy; // S22 (j-1/2)
          break;
        case DiffusionDir::z: // S23
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (v(i, j, k+1) - v(i, j, k))/dz + (w(i, j, k+1) - w(i, j-1, k+1))/dy; // S23 (k+1/2) //TODO: Check this with Branko
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (v(i, j, k) - v(i, j, k-1))/dz + (w(i, j, k) - w(i, j-1, k))/dy; // S23 (k-1/2) //TODO: Check this with Branko
          stressTerm*= 0.5;
          break;
        default:
          amrex::Abort("Error: Diffusion direction is unrecognized");
      }
      break;
    case MomentumEqn::z:
      switch (diffDir) {
        case DiffusionDir::x: // S31
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (u(i+1, j, k) - u(i+1, j, k-1))/dz + (w(i+1, j, k) - w(i, j, k))/dx; // S31 (i+1/2)
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (u(i, j, k) - u(i, j, k-1))/dz + (w(i, j, k) - w(i-1, j, k))/dx; // S31 (i-1/2)
          stressTerm*= 0.5;
          break;
        case DiffusionDir::y: // S32
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (v(i, j+1, k) - v(i, j+1, k-1))/dz + (w(i, j+1, k) - w(i, j, k))/dy; // S32 (j+1/2) //TODO: Check this with Branko
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (v(i, j, k) - v(i, j, k-1))/dz + (w(i, j, k) - w(i, j-1, k))/dy; // S32 (j-1/2) //TODO: Check this with Branko
          stressTerm*= 0.5;
          break;
        case DiffusionDir::z: // S33
          if (nextOrPrev == NextOrPrev::next)
            stressTerm = (w(i, j, k+1) - w(i, j, k))/dz; // S33 (k+1/2)
          else // nextOrPrev == NextOrPrev::prev
            stressTerm = (w(i, j, k) - w(i, j, k-1))/dz; // S33 (k-1/2)
          break;
        default:
          amrex::Abort("Error: Diffusion direction is unrecognized");
      }
      break;
    default:
      amrex::Abort("Error: Momentum equation is unrecognized");
  }

  return stressTerm;
}


// Compute tau_ij
Real ComputeStressTermSmag (const int &i, const int &j, const int &k,
                            const Array4<Real>& u, const Array4<Real>& v, const Array4<Real>& w,
                            const enum NextOrPrev &nextOrPrev,
                            const enum MomentumEqn &momentumEqn,
                            const enum DiffusionDir &diffDir,
                            const Geometry &geom,
                            const Array4<Real>& nut) {

  auto stressTerm = ComputeStressTerm(i, j, k, u, v, w, nextOrPrev, momentumEqn, diffDir, geom); // S_ij (next or prev)
  Real K_turb = 1.0;

  switch (momentumEqn) {
    case MomentumEqn::x:
      switch (diffDir) {
        case DiffusionDir::x: // tau11
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i, j, k) needed to obtain tau11 (i+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i-1, j, k); // K (i-1, j, k) needed to obtain tau11 (i-1/2)
          break;
        case DiffusionDir::y: // tau12
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i-1/2, j+1/2, k) needed to obtain tau12 (j+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j, k); // K (i-1/2, j-1/2, k) needed to obtain tau12 (j-1/2)
          break;
        case DiffusionDir::z: // tau13
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i-1/2, j, k+1/2) needed to obtain tau13 (k+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j, k); // K (i-1/2, j, k-1/2) needed to obtain tau13 (k-1/2)
          break;
        default:
          amrex::Abort("Error: Diffusion direction is unrecognized");
      }
      break;
    case MomentumEqn::y:
      switch (diffDir) {
        case DiffusionDir::x: // tau21
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i+1/2, j-1/2, k) needed to obtain tau21 (i+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j, k); // K (i-1/2, j-1/2, k) needed to obtain tau21 (i-1/2)
          break;
        case DiffusionDir::y: // tau22
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i, j, k) needed to obtain tau22 (j+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j-1, k); // K (i, j-1, k) needed to obtain tau22 (j-1/2)
          break;
        case DiffusionDir::z: // tau23
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i, j-1/2, k+1/2) needed to obtain tau23 (k+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j, k); // K (i, j-1/2, k-1/2) needed to obtain tau23 (k-1/2)
          break;
        default:
          amrex::Abort("Error: Diffusion direction is unrecognized");
      }
      break;
    case MomentumEqn::z:
      switch (diffDir) {
        case DiffusionDir::x: // tau31
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i+1/2, j, k-1/2) needed to obtain tau31 (i+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j, k); // K (i-1/2, j, k-1/2) needed to obtain tau31 (i-1/2)
          break;
        case DiffusionDir::y: // tau32
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i, j+1/2, k-1/2) needed to obtain tau32 (j+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j, k); // K (i, j-1/2, k-1/2) needed to obtain tau32 (j-1/2)
          break;
        case DiffusionDir::z: // tau33
          if (nextOrPrev == NextOrPrev::next)
            K_turb = nut(i, j, k); // K (i, j, k) needed to obtain tau33 (k+1/2)
          else // nextOrPrev == NextOrPrev::prev
            K_turb = nut(i, j, k-1); // K (i, j, k-1) needed to obtain tau33 (k-1/2)
          break;
        default:
          amrex::Abort("Error: Diffusion direction is unrecognized");
      }
      break;
    default:
      amrex::Abort("Error: Momentum equation is unrecognized");
  }

  return K_turb*stressTerm;
}

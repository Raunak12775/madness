{
  stdenv,
  cmake,
  mkl,
  mpi,
  python311,
  pkg-config,
  libxc,
}:
stdenv.mkDerivation {
  name = "madness";
  nativeBuildInputs = [cmake mkl mpi python311 pkg-config libxc];
  src = ../.;
}

! RUN: %S/test_errors.sh %s %t %f18 -fopenmp
! Check OpenMP 5.0 - 2.17.8 flush Construct

!  use omp_lib
  implicit none

  integer :: i, a, b
  real, DIMENSION(10) :: array, arraya
  a = 1.0
  array = (/1, 2, 3, 4, 5, 6, 7, 8, 9, 10/)
  !$omp parallel num_threads(4)

   !syntax error - yes we get syntax error
!  !$omp flush ACQ_REL, ACQ_REL

  !allowed - yes test passed parser.
  !$omp flush ACQ_REL
  !allowed
  !$omp flush RELEASE

!  not allowed
  !$omp flush SEQ_CST
!  not allowed
  !$omp flush RELAXED
  !$omp flush PRIVATE(a)
  !$omp flush SIMDLEN(10)
  !$omp flush NUM_THREADS(4)
  !$omp flush COLLAPSE(1)
  !$omp flush ALLOCATE(a)
  !$omp flush COPYIN(a)
  !$omp flush DEFAULT(private)
  !$omp flush DEFAULTMAP(tofrom:scalar)
  !$omp flush DEVICE(0)
  !$omp flush DIST_SCHEDULE(static, 2)
  !$omp flush FIRSTPRIVATE(b)
  !$omp flush FROM(arraya)

!$omp end parallel
end

! RUN: %S/test_errors.sh %s %t %f18 -fopenmp
! Check OpenMP 5.0 - 2.17.8 flush Construct

  use omp_lib
  implicit none

  TYPE someStruct
    REAL :: rr
  end TYPE
  integer :: i, a, b
  real, DIMENSION(10) :: array
  TYPE(someStruct) :: structObj

  a = 1.0
  !$omp parallel num_threads(4)
  !No list flushes all.
  if (omp_get_thread_num() == 1) THEN
    !$omp flush
  END IF






  array = (/1, 2, 3, 4, 5, 6, 7, 8, 9, 10/)
!  !syntax error - yes we get syntax error
!  !$omp flush ACQ_REL, ACQ_REL
   !allowed - yes test passed parser.
!  !$omp flush ACQ_REL
!  !allowed
  !$omp flush SEQ_CST
!  !allowed
!  !$omp flush RELEASE
!  !syntax error
  !$omp flush RELAXED
!  !syntax error
!  !$omp flush PRIVATE


  !$omp end parallel
end

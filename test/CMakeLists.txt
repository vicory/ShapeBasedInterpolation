itk_module_test()
set(ShapeBasedInterpolationTests
  ShapeBasedInterpolationTest.cxx
)

set(TEMP ${CMAKE_BINARY_DIR}/Testing/Temporary)

CreateTestDriver(ShapeBasedInterpolation  "${ShapeBasedInterpolation-Test_LIBRARIES}" "${ShapeBasedInterpolationTests}")

itk_add_test(NAME ShapeBasedInterpolationTest
      COMMAND ShapeBasedInterpolationTestDriver --without-threads
      --compare
      ${ITK_TEST_OUTPUT_DIR}/ShapeBasedInterpolationTestOutput.mha DATA{Baseline/cone_after_interpolation.mha}
      ShapeBasedInterpolationTest DATA{Input/cone_before_interpolation.mha} ${ITK_TEST_OUTPUT_DIR}/ShapeBasedInterpolationTestOutput.mha 
)


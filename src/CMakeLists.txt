add_library( ShapeBasedInterpolation ${ITK_LIBRARY_BUILD_TYPE} itkInterBinaryShapeBasedInterpolationImageFilter.cxx itkIntraBinaryShapeBasedInterpolationImageFilter.cxx )

target_link_libraries(ShapeBasedInterpolation ${ITKCommon_LIBRARIES})
itk_module_target(ShapeBasedInterpolation)




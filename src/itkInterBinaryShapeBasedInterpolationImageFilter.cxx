/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#include "itkInterBinaryShapeBasedInterpolationImageFilter.h"
#include <itkLinearInterpolateImageFunction.h>

namespace itk
{  
/**
   * Constructor
   */
InterBinaryShapeBasedInterpolationImageFilter::InterBinaryShapeBasedInterpolationImageFilter()
{
  m_DelineationRatio = 3; 
  m_DelineationZCoordinateArray.clear();
    
  m_IntermediateImage = IntermediateImageType::New();
    
  m_SliceBySliceFilter = SliceBySliceFilterType::New();
  m_DistanceMapImageFilter = DistanceMapImageFilterType::New();
    
  m_ResampleFilter = ResampleFilterType::New();  
  m_ResampleFilter->SetDefaultPixelValue( 1 );
  m_Transform = TransformType::New();
  m_Interpolator = LinearInterpolateImageFunction< IntermediateImageType2, double >::New();   
}
  
/**
   * Destructor
   */
InterBinaryShapeBasedInterpolationImageFilter::~InterBinaryShapeBasedInterpolationImageFilter()
{
}
  
  
  
/**
   * Standard PrintSelf method. Print out a description of self. Print the filter parameters.
   */
void InterBinaryShapeBasedInterpolationImageFilter::PrintSelf( std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os, indent);
  os << indent << "DelineationRatio" << m_DelineationRatio << std::endl;
  os << indent << "Interpolator: " << m_Interpolator.GetPointer() << std::endl;
}  
  
  
  
void InterBinaryShapeBasedInterpolationImageFilter::FindZCoordinatesOfDelineatedSlices()
{
  std::cout << "Appel fonction FindZCoordinatesOfDelineatedSlices()" << std::endl;
  InputImageConstPointer inputImage = this->GetInput();
  InputRegionType inputRegion = inputImage->GetLargestPossibleRegion();
    
  ConstSliceItType sliceIt( inputImage, inputRegion );
  sliceIt.SetFirstDirection(0);
  sliceIt.SetSecondDirection(1);
  sliceIt.GoToBegin(); 
    
  InputIndexType inputStart;   
  inputStart = sliceIt.GetIndex();
    
  InputPixelType pixel;
  InputPixelType pixelSum;
  
  // Count the number of slices that have been delineated in the input
  // image and store their z coordinate in a vector using the STL
  // library (std::vector)
  while( !sliceIt.IsAtEnd() )
    {
    pixelSum = 0;
    while( !sliceIt.IsAtEndOfSlice() )
      {
      while( !sliceIt.IsAtEndOfLine() )
        {
        pixel = sliceIt.Get();
        pixelSum += pixel;
        ++sliceIt;
        }
      sliceIt.NextLine();
      }
    if ( pixelSum > 0 )   // it means that the slice is delineated since there is at least one non-zero pixel intensity 
      {
      m_DelineationZCoordinateArray.push_back( sliceIt.GetIndex()[2] );
      }   
    sliceIt.NextSlice();
    }
 
  std::cout << "The first delineated slice is at z : " << m_DelineationZCoordinateArray.front() << std::endl;
  std::cout << "The last delineated slice is at z : " << m_DelineationZCoordinateArray.back() << std::endl;
}
  
  
  
void InterBinaryShapeBasedInterpolationImageFilter::GenerateIntermediateImageInformation()
{ 
  std::cout << "Appel fonction GenerateIntermediateImageInformation()" << std::endl;
  InputImageConstPointer inputImage = this->GetInput();
     
  //Define the spacing of the intermediate image 
  const InputImageType::SpacingType& inputSpacing = inputImage->GetSpacing();
  IntermediateImageType::SpacingType intermediateSpacing;
  intermediateSpacing[0] = inputSpacing[0];
  intermediateSpacing[1] = inputSpacing[1];
  intermediateSpacing[2] = m_DelineationRatio * inputSpacing[2];
    
  //Define the starting index of the intermediate regions 
  IntermediateIndexType intermediateStart;
  IntermediateIndexType physicalIntermediateStart;
    
  intermediateStart.Fill(0);
    
  physicalIntermediateStart[0] = 0;
  physicalIntermediateStart[1] = 0;
  physicalIntermediateStart[2] = m_DelineationZCoordinateArray.front();
    
  //Define the size of the intermediate regions 
  InputRegionType inputRegion = inputImage->GetLargestPossibleRegion();
  InputSizeType inputSize = inputRegion.GetSize();
  IntermediateSizeType intermediateSize;
  intermediateSize[0] = inputSize[0];
  intermediateSize[1] = inputSize[1];
  intermediateSize[2] = m_DelineationZCoordinateArray.size();
  
  IntermediateRegionType intermediateRegion( intermediateStart, intermediateSize );
    
  //Define the origin of the intermediate image 
  const InputPointType& inputOrigin = inputImage->GetOrigin();
  IntermediatePointType intermediateOrigin;
    
  for(unsigned int i=0; i< ImageDimension; i++)
    {
    intermediateOrigin[i] = inputOrigin[i] + inputSpacing[i] * physicalIntermediateStart[i];
    }
    
  m_IntermediateImage->SetRegions( intermediateRegion );
  m_IntermediateImage->SetSpacing( intermediateSpacing );
  m_IntermediateImage->SetOrigin( intermediateOrigin );
  m_IntermediateImage->Allocate(); 
}
  

  
void InterBinaryShapeBasedInterpolationImageFilter::GenerateData()
{
  std::cout << "Appel fonction GenerateData()" << std::endl;
 
  this->FindZCoordinatesOfDelineatedSlices();
  this->GenerateIntermediateImageInformation();
    
  InputImageConstPointer inputImage = this->GetInput(); 
  InputRegionType inputRegion = inputImage->GetLargestPossibleRegion();
  InputSizeType inputSize = inputRegion.GetSize();
  const InputSpacingType& inputSpacing = inputImage->GetSpacing();
  const InputPointType& inputOrigin = inputImage->GetOrigin();
  ConstRegionItType inputIt( inputImage, inputRegion );
    
  IntermediateRegionType intermediateRegion = m_IntermediateImage->GetLargestPossibleRegion();
  RegionItType intermediateIt( m_IntermediateImage, intermediateRegion );
     
  OutputImagePointer outputImage = this->GetOutput(); 
    
    
  // Copy the slices that are delineated into the m_IntermediateImage 
  for( inputIt.GoToBegin(), intermediateIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt )
    {
    if( std::find(m_DelineationZCoordinateArray.begin(), m_DelineationZCoordinateArray.end(), inputIt.GetIndex()[2]) != m_DelineationZCoordinateArray.end() )
      {
      intermediateIt.Set( inputIt.Get() );
      ++intermediateIt;
      }
    }
    
    
  // Resample the m_IntermediateImage   
  m_SliceBySliceFilter->SetFilter( m_DistanceMapImageFilter );
  m_SliceBySliceFilter->SetInput( m_IntermediateImage ); 
    
  m_ResampleFilter->SetInput( m_SliceBySliceFilter->GetOutput() );
  m_ResampleFilter->SetTransform( m_Transform );
  m_ResampleFilter->SetInterpolator( m_Interpolator );
  m_ResampleFilter->SetSize( inputSize ); 
  m_ResampleFilter->SetOutputSpacing( inputSpacing ); 
  m_ResampleFilter->SetOutputOrigin( inputOrigin );   
  m_ResampleFilter->Update();
    
    
  IntermediateRegionItType itr( m_ResampleFilter->GetOutput(), m_ResampleFilter->GetOutput()->GetLargestPossibleRegion() );

  OutputRegionType outputRegion = m_ResampleFilter->GetOutput()->GetLargestPossibleRegion();
  outputImage->SetRegions( outputRegion );
  outputImage->CopyInformation( m_ResampleFilter->GetOutput() );
  outputImage->Allocate();
    
  OutputRegionItType outItr( outputImage, outputRegion );
 
  // Threshold the output image to get a binary interpolated image
  for( itr.GoToBegin(), outItr.GoToBegin(); !itr.IsAtEnd(); ++itr, ++outItr )
    {
    if( itr.Get() > 0 ) 
      {
      if( floor(10*itr.Get())/10 > 0)
        {
        outItr.Set(0);
        }
      else
        {
        outItr.Set(1);
        }      }
    else
      {
      outItr.Set(1);
      }
    }
}
} // end namespace itk

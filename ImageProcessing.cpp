 
//**************************************************************************
//* WARNING: This file was automatically generated.  Any changes you make  *
//*          to this file will be lost if you generate the file again.     *
//**************************************************************************
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nivision.h>
#include <cstdio>
#include "ImageProcessing.h"

// If you call Machine Vision functions in your script, add NIMachineVision.c to the project.

static IVA_Data* IVA_InitData(int numSteps, int numCoordSys);
static int IVA_DisposeStepResults(IVA_Data* ivaData, int stepIndex);
static int IVA_CLRThreshold(Image* image, int min1, int max1, int min2, int max2, int min3, int max3, int colorMode);
static int IVA_Particle(Image* image,
                                 int connectivity,
                                 int pPixelMeasurements[],
                                 int numPixelMeasurements,
                                 int pCalibratedMeasurements[],
                                 int numCalibratedMeasurements,
                                 IVA_Data* ivaData,
                                 int stepIndex);
static int IVA_DetectRectangles(Image* image,
                                         IVA_Data* ivaData,
                                         double minWidth,
                                         double maxWidth,
                                         double minHeight,
                                         double maxHeight, 
                                         int extraction,
                                         int curveThreshold,
                                         int edgeFilterSize,
                                         int curveMinLength,
                                         int curveRowStepSize,
                                         int curveColumnStepSize,
                                         int curveMaxEndPointGap,
                                         int matchMode, 
                                         float rangeMin[],
                                         float rangeMax[],
                                         float score,
                                         ROI* roi,
                                         int stepIndex);

int IVA_ProcessImage(Image *image, IVA_Data* &ivaData)
{
	int success = 1;
    int pPixelMeasurements[81] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,35,36,
        37,38,39,41,42,43,45,46,48,49,50,51,53,54,55,56,58,59,60,61,
        62,63,64,65,66,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,
        83,84,85,86,87,88};
    int *pCalibratedMeasurements = 0;
    int pKernel[9] = {1,1,1,1,1,1,1,1,1};
    StructuringElement structElem;
    ROI *roi;
    float rangeMin[3] = {0,0,50};
    float rangeMax[3] = {360,0,200};

    // Initializes internal data (buffers and array of points for caliper measurements)
    VisionErrChk(ivaData = IVA_InitData(6, 0));

	VisionErrChk(IVA_CLRThreshold(image, 231, 19, 45, 255, 90, 207, IMAQ_HSL));

	VisionErrChk(IVA_Particle(image, TRUE, pPixelMeasurements, 81, 
		pCalibratedMeasurements, 0, ivaData, 1));

    //-------------------------------------------------------------------//
    //                  Advanced Morphology: Convex Hull                 //
    //-------------------------------------------------------------------//

    // Computes the convex envelope for each labeled particle in the source image.
    VisionErrChk(imaqConvexHull(image, image, TRUE));

    //-------------------------------------------------------------------//
    //                Advanced Morphology: Remove Objects                //
    //-------------------------------------------------------------------//

    structElem.matrixCols = 3;
    structElem.matrixRows = 3;
    structElem.hexa = FALSE;
    structElem.kernel = pKernel;

    // Filters particles based on their size.
    VisionErrChk(imaqSizeFilter(image, image, TRUE, 1, (SizeType)0, &structElem));

    //-------------------------------------------------------------------//
    //                  Advanced Morphology: Convex Hull                 //
    //-------------------------------------------------------------------//

    // Computes the convex envelope for each labeled particle in the source image.
    VisionErrChk(imaqConvexHull(image, image, TRUE));

    // Creates a new, empty region of interest.
    VisionErrChk(roi = imaqCreateROI());

    //-------------------------------------------------------------------//
    //                            Detect Rectangles                      //
    //-------------------------------------------------------------------//

	VisionErrChk(IVA_DetectRectangles(image, ivaData, 10, 400, 10, 300, 
		IMAQ_NORMAL_IMAGE, 1, IMAQ_NORMAL, 10, 15, 15, 10, 5, rangeMin, 
		rangeMax, 100, roi, 5));

    // Cleans up resources associated with the object
    imaqDispose(roi);

Error:
	return success;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_CLRThreshold
//
// Description  : Thresholds a color image.
//
// Parameters   : image      -  Input image
//                min1       -  Minimum range for the first plane
//                max1       -  Maximum range for the first plane
//                min2       -  Minimum range for the second plane
//                max2       -  Maximum range for the second plane
//                min3       -  Minimum range for the third plane
//                max3       -  Maximum range for the third plane
//                colorMode  -  Color space in which to perform the threshold
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static int IVA_CLRThreshold(Image* image, int min1, int max1, int min2, int max2, int min3, int max3, int colorMode)
{
    int success = 1;
    Image* thresholdImage;
    Range plane1Range;
    Range plane2Range;
    Range plane3Range;


    //-------------------------------------------------------------------//
    //                          Color Threshold                          //
    //-------------------------------------------------------------------//

    // Creates an 8 bit image for the thresholded image.
    VisionErrChk(thresholdImage = imaqCreateImage(IMAQ_IMAGE_U8, 7));

    // Set the threshold range for the 3 planes.
    plane1Range.minValue = min1;
    plane1Range.maxValue = max1;
    plane2Range.minValue = min2;
    plane2Range.maxValue = max2;
    plane3Range.minValue = min3;
    plane3Range.maxValue = max3;

    // Thresholds the color image.
    VisionErrChk(imaqColorThreshold(thresholdImage, image, 1, (ColorMode)colorMode, &plane1Range, &plane2Range, &plane3Range));

    // Copies the threshold image in the souce image.
    VisionErrChk(imaqDuplicate(image, thresholdImage));

Error:
    imaqDispose(thresholdImage);

    return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_Particle
//
// Description  : Computes the number of particles detected in a binary image and
//                a 2D array of requested measurements about the particle.
//
// Parameters   : image                      -  Input image
//                connectivity               -  Set this parameter to 1 to use
//                                              connectivity-8 to determine
//                                              whether particles are touching.
//                                              Set this parameter to 0 to use
//                                              connectivity-4 to determine
//                                              whether particles are touching.
//                pixelMeasurements          -  Array of measuremnets parameters
//                numPixelMeasurements       -  Number of elements in the array
//                calibratedMeasurements     -  Array of measuremnets parameters
//                numCalibratedMeasurements  -  Number of elements in the array
//                ivaData                    -  Internal Data structure
//                stepIndex                  -  Step index (index at which to store
//                                              the results in the resuts array)
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static int IVA_Particle(Image* image,
                                 int connectivity,
                                 int pPixelMeasurements[],
                                 int numPixelMeasurements,
                                 int pCalibratedMeasurements[],
                                 int numCalibratedMeasurements,
                                 IVA_Data* ivaData,
                                 int stepIndex)
{
    int success = 1;
    int numParticles;
    double* pixelMeasurements = NULL;
    double* calibratedMeasurements = NULL;
    unsigned int visionInfo;
    IVA_Result* particleResults;
    int i;
    int j;
    double centerOfMassX;
    double centerOfMassY;


    //-------------------------------------------------------------------//
    //                         Particle Analysis                         //
    //-------------------------------------------------------------------//

    // Counts the number of particles in the image.
    VisionErrChk(imaqCountParticles(image, connectivity, &numParticles));

    // Allocate the arrays for the measurements.
    pixelMeasurements = (double*)malloc(numParticles * numPixelMeasurements * sizeof(double));
    calibratedMeasurements = (double*)malloc(numParticles * numCalibratedMeasurements * sizeof(double));

    // Delete all the results of this step (from a previous iteration)
    IVA_DisposeStepResults(ivaData, stepIndex);

    // Check if the image is calibrated.
    VisionErrChk(imaqGetVisionInfoTypes(image, &visionInfo));

    // If the image is calibrated, we also need to log the calibrated position (x and y)
    ivaData->stepResults[stepIndex].numResults = (visionInfo & IMAQ_VISIONINFO_CALIBRATION ?
                                                  numParticles * 4 + 1 : numParticles * 2 + 1);
    ivaData->stepResults[stepIndex].results = (IVA_Result*)malloc (sizeof(IVA_Result) * ivaData->stepResults[stepIndex].numResults);
    
    particleResults = ivaData->stepResults[stepIndex].results;

    #if defined (IVA_STORE_RESULT_NAMES)
        sprintf(particleResults->resultName, "Object #");
    #endif
    particleResults->type = IVA_NUMERIC;
    particleResults->resultVal.numVal = numParticles;
    particleResults++;
    
    for (i = 0 ; i < numParticles ; i++)
    {
        // Computes the requested pixel measurements about the particle.
        for (j = 0 ; j < numPixelMeasurements ; j++)
        {
            VisionErrChk(imaqMeasureParticle(image, i, FALSE, (MeasurementType)pPixelMeasurements[j], &pixelMeasurements[i*numPixelMeasurements + j]));
        }

        // Computes the requested calibrated measurements about the particle.
        for (j = 0 ; j < numCalibratedMeasurements ; j++)
        {
            VisionErrChk(imaqMeasureParticle(image, i, TRUE, (MeasurementType)pCalibratedMeasurements[j], &calibratedMeasurements[i*numCalibratedMeasurements + j]));
        }
        
        #if defined (IVA_STORE_RESULT_NAMES)
            sprintf(particleResults->resultName, "Particle %d.X Position (Pix.)", i + 1);
        #endif
        particleResults->type = IVA_NUMERIC;
        VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_X, &centerOfMassX));
        particleResults->resultVal.numVal = centerOfMassX;
        particleResults++;

        #if defined (IVA_STORE_RESULT_NAMES)
            sprintf(particleResults->resultName, "Particle %d.Y Position (Pix.)", i + 1);
        #endif
        particleResults->type = IVA_NUMERIC;
        VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_Y, &centerOfMassY));
        particleResults->resultVal.numVal = centerOfMassY;
        particleResults++;

        if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
        {
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(particleResults->resultName, "Particle %d.X Position (Calibrated)", i + 1);
            #endif
            particleResults->type = IVA_NUMERIC;
            VisionErrChk(imaqMeasureParticle(image, i, TRUE, IMAQ_MT_CENTER_OF_MASS_X, &centerOfMassX));
            particleResults->resultVal.numVal = centerOfMassX;
            particleResults++;

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(particleResults->resultName, "Particle %d.Y Position (Calibrated)", i + 1);
            #endif
            particleResults->type = IVA_NUMERIC;
            VisionErrChk(imaqMeasureParticle(image, i, TRUE, IMAQ_MT_CENTER_OF_MASS_Y, &centerOfMassY));
            particleResults->resultVal.numVal = centerOfMassY;
            particleResults++;
        }
    }

Error:
    free(pixelMeasurements);
    free(calibratedMeasurements);

    return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_DetectRectangles
//
// Description  : Searches for rectangles in an image that are within a given range
//
// Parameters   : image                -  Input image
//                ivaData              -  Internal Data structure
//                minWidth             -  Minimum Width
//                maxWidth             -  Maximum Width
//                minHeight            -  Minimum Height
//                maxHeight            -  Maximum Height
//                extraction           -  Extraction mode
//                curveThreshold       -  Specifies the minimum contrast at a
//                                        pixel for it to be considered as part
//                                        of a curve.
//                edgeFilterSize       -  Specifies the width of the edge filter
//                                        the function uses to identify curves in
//                                        the image.
//                curveMinLength       -  Specifies the smallest curve the
//                                        function will identify as a curve.
//                curveRowStepSize     -  Specifies the distance, in the x direction
//                                        between two pixels the function inspects
//                                        for curve seed points.
//                curveColumnStepSize  -  Specifies the distance, in the y direction,
//                                        between two pixels the function inspects
//                                        for curve seed points.
//                curveMaxEndPointGap  -  Specifies the maximum gap, in pixels,
//                                        between the endpoints of a curve that the
//                                        function identifies as a closed curve.
//                matchMode            -  Specifies the method to use when looking
//                                        for the pattern in the image.
//                rangeMin             -  Match constraints range min array
//                                        (angle 1, angle 2, scale, occlusion)
//                rangeMax             -  Match constraints range max array
//                                        (angle 1, angle 2, scale, occlusion)
//                score                -  Minimum score a match can have for the
//                                        function to consider the match valid.
//                roi                  -  Search area
//                stepIndex            -  Step index (index at which to store the results in the resuts array)
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static int IVA_DetectRectangles(Image* image,
                                         IVA_Data* ivaData,
                                         double minWidth,
                                         double maxWidth,
                                         double minHeight,
                                         double maxHeight, 
                                         int extraction,
                                         int curveThreshold,
                                         int edgeFilterSize,
                                         int curveMinLength,
                                         int curveRowStepSize,
                                         int curveColumnStepSize,
                                         int curveMaxEndPointGap,
                                         int matchMode, 
                                         float rangeMin[],
                                         float rangeMax[],
                                         float score,
                                         ROI* roi,
                                         int stepIndex)
{
    int success = 1;
    RectangleDescriptor rectangleDescriptor;     
    CurveOptions curveOptions;
    ShapeDetectionOptions shapeOptions; 
    RangeFloat orientationRange[2]; 
    int i;
    RectangleMatch* matchedRectangles = NULL; 
    int numMatchesFound;
    int numObjectResults;
    IVA_Result* shapeMacthingResults;
    unsigned int visionInfo;
    TransformReport* realWorldPosition = NULL;
    float calibratedWidth;
    float calibratedHeight;


    //-------------------------------------------------------------------//
    //                        Detect Rectangles                          //
    //-------------------------------------------------------------------//

    // Fill in the Curve options.
    curveOptions.extractionMode = (ExtractionMode)extraction;
    curveOptions.threshold = curveThreshold;
    curveOptions.filterSize = (EdgeFilterSize)edgeFilterSize;
    curveOptions.minLength = curveMinLength;
    curveOptions.rowStepSize = curveRowStepSize;
    curveOptions.columnStepSize = curveColumnStepSize;
    curveOptions.maxEndPointGap = curveMaxEndPointGap;
    curveOptions.onlyClosed = 0;
    curveOptions.subpixelAccuracy = 0;

    rectangleDescriptor.minWidth = minWidth;
    rectangleDescriptor.maxWidth = maxWidth;
    rectangleDescriptor.minHeight = minHeight;
    rectangleDescriptor.maxHeight = maxHeight;

    for (i = 0 ; i < 2 ; i++)
    {
        orientationRange[i].minValue = rangeMin[i];
        orientationRange[i].maxValue = rangeMax[i];
    }

    shapeOptions.mode = matchMode;
    shapeOptions.angleRanges = orientationRange;
    shapeOptions.numAngleRanges = 2;
    shapeOptions.scaleRange.minValue = rangeMin[2];
    shapeOptions.scaleRange.maxValue = rangeMax[2];
    shapeOptions.minMatchScore = score;

    matchedRectangles = NULL;
    numMatchesFound = 0;

    // Searches for rectangles in the image that are within the range.
    VisionErrChk(matchedRectangles = imaqDetectRectangles(image, &rectangleDescriptor, &curveOptions, &shapeOptions, roi, &numMatchesFound));

    // ////////////////////////////////////////
    // Store the results in the data structure.
    // ////////////////////////////////////////
    
    // First, delete all the results of this step (from a previous iteration)
    IVA_DisposeStepResults(ivaData, stepIndex);

    // Check if the image is calibrated.
    VisionErrChk(imaqGetVisionInfoTypes(image, &visionInfo));

    // If the image is calibrated, we also need to log the calibrated position (x and y) -> 22 results instead of 12
    numObjectResults = (visionInfo & IMAQ_VISIONINFO_CALIBRATION ? 22 : 12);

    ivaData->stepResults[stepIndex].numResults = numMatchesFound * numObjectResults + 1;
    ivaData->stepResults[stepIndex].results = (IVA_Result*)malloc (sizeof(IVA_Result) * ivaData->stepResults[stepIndex].numResults);
    shapeMacthingResults = ivaData->stepResults[stepIndex].results;
    
    if (shapeMacthingResults)
    {
        #if defined (IVA_STORE_RESULT_NAMES)
            sprintf(shapeMacthingResults->resultName, "# Matches");
        #endif
        shapeMacthingResults->type = IVA_NUMERIC;
        shapeMacthingResults->resultVal.numVal = numMatchesFound;
        shapeMacthingResults++;
        
        for (i = 0 ; i < numMatchesFound ; i++)
        {
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Score", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].score;
            shapeMacthingResults++;
            
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Width (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].width;
            shapeMacthingResults++;
            
            if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
            {
                realWorldPosition = imaqTransformPixelToRealWorld(image, matchedRectangles[i].corner, 4);
                imaqGetDistance (realWorldPosition->points[0], realWorldPosition->points[1], &calibratedWidth);
                imaqGetDistance (realWorldPosition->points[1], realWorldPosition->points[2], &calibratedHeight);
                
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Width (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = calibratedWidth;
                shapeMacthingResults++;
            }

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Height (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].height;
            shapeMacthingResults++;
            
            if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
            {
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Height (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = calibratedHeight;
                shapeMacthingResults++;
            }
            
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Angle (degrees)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].rotation;
            shapeMacthingResults++;

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner1 X (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[0].x;
            shapeMacthingResults++;

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner1 Y (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[0].y;
            shapeMacthingResults++;
            
            if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
            {
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner1 X (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[0].x;
                shapeMacthingResults++;
                
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner1 Y (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[0].y;
                shapeMacthingResults++;
            }
            
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner2 X (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[1].x;
            shapeMacthingResults++;

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner2 Y (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[1].y;
            shapeMacthingResults++;
            
            if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
            {
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner2 X (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[1].x;
                shapeMacthingResults++;
                
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner2 Y (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[1].y;
                shapeMacthingResults++;
            }
            
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner3 X (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[2].x;
            shapeMacthingResults++;

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner3 Y (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[2].y;
            shapeMacthingResults++;
            
            if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
            {
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner3 X (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[2].x;
                shapeMacthingResults++;
                
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner3 Y (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[2].y;
                shapeMacthingResults++;
            }
            
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner4 X (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[3].x;
            shapeMacthingResults++;

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner4 Y (Pix.)", i + 1);
            #endif
            shapeMacthingResults->type = IVA_NUMERIC;
            shapeMacthingResults->resultVal.numVal = matchedRectangles[i].corner[3].y;
            shapeMacthingResults++;
            
            if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
            {
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner4 X (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[3].x;
                shapeMacthingResults++;
                
                #if defined (IVA_STORE_RESULT_NAMES)
                    sprintf(shapeMacthingResults->resultName, "Rectangle %d.Corner4 Y (World)", i + 1);
                #endif
                shapeMacthingResults->type = IVA_NUMERIC;
                shapeMacthingResults->resultVal.numVal = realWorldPosition->points[3].y;
                shapeMacthingResults++;
            }
        }
    }

Error:
    // Disposes temporary structures.
    imaqDispose(matchedRectangles);
    imaqDispose(realWorldPosition);

    return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_InitData
//
// Description  : Initializes data for buffer management and results.
//
// Parameters   : # of steps
//                # of coordinate systems
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static IVA_Data* IVA_InitData(int numSteps, int numCoordSys)
{
    int success = 1;
    IVA_Data* ivaData = NULL;
    int i;


    // Allocate the data structure.
    VisionErrChk(ivaData = (IVA_Data*)malloc(sizeof (IVA_Data)));

    // Initializes the image pointers to NULL.
    for (i = 0 ; i < IVA_MAX_BUFFERS ; i++)
        ivaData->buffers[i] = NULL;

    // Initializes the steo results array to numSteps elements.
    ivaData->numSteps = numSteps;

    ivaData->stepResults = (IVA_StepResults*)malloc(ivaData->numSteps * sizeof(IVA_StepResults));
    for (i = 0 ; i < numSteps ; i++)
    {
        #if defined (IVA_STORE_RESULT_NAMES)
            sprintf(ivaData->stepResults[i].stepName, " ");
        #endif
        ivaData->stepResults[i].numResults = 0;
        ivaData->stepResults[i].results = NULL;
    }

    // Create the coordinate systems
	ivaData->baseCoordinateSystems = NULL;
	ivaData->MeasurementSystems = NULL;
	if (numCoordSys)
	{
		ivaData->baseCoordinateSystems = (CoordinateSystem*)malloc(sizeof(CoordinateSystem) * numCoordSys);
		ivaData->MeasurementSystems = (CoordinateSystem*)malloc(sizeof(CoordinateSystem) * numCoordSys);
	}

    ivaData->numCoordSys = numCoordSys;

Error:
    return ivaData;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_DisposeData
//
// Description  : Releases the memory allocated in the IVA_Data structure
//
// Parameters   : ivaData  -  Internal data structure
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
int IVA_DisposeData(IVA_Data* ivaData)
{
    int i;


    // Releases the memory allocated for the image buffers.
    for (i = 0 ; i < IVA_MAX_BUFFERS ; i++)
        imaqDispose(ivaData->buffers[i]);

    // Releases the memory allocated for the array of measurements.
    for (i = 0 ; i < ivaData->numSteps ; i++)
        IVA_DisposeStepResults(ivaData, i);

    free(ivaData->stepResults);

    // Dispose of coordinate systems
    if (ivaData->numCoordSys)
    {
        free(ivaData->baseCoordinateSystems);
        free(ivaData->MeasurementSystems);
    }

    free(ivaData);

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_DisposeStepResults
//
// Description  : Dispose of the results of a specific step.
//
// Parameters   : ivaData    -  Internal data structure
//                stepIndex  -  step index
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static int IVA_DisposeStepResults(IVA_Data* ivaData, int stepIndex)
{
    int i;

    
    for (i = 0 ; i < ivaData->stepResults[stepIndex].numResults ; i++)
    {
        if (ivaData->stepResults[stepIndex].results[i].type == IVA_STRING)
            free(ivaData->stepResults[stepIndex].results[i].resultVal.strVal);
    }

    free(ivaData->stepResults[stepIndex].results);

    return TRUE;
}



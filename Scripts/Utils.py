import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
import math
import importlib.util
import os

def FindImageFilesAndDir(subdir="", suffixFilter="jpg"):
    dir = "../test/images"
    if len(subdir) > 0:
        dir = "{0}/{1}".format(dir, subdir)
    files = os.listdir(dir)
    suffix = "." + suffixFilter
    list = []
    for file in files:
        if file.endswith(suffix):
            filename = str("{0}/{1}".format(dir, file))
            #filename = os.path.abspath(filename)
            list.append(filename)
    return list, dir

def ImgShow(images, resolution=120):
    plt.rcParams['figure.dpi'] = resolution
    
    cnt = len(images)
    if cnt > 20:
        # Top-level object is an image, make it an array of 1
        images = [images]
        cnt = len(images)
        
    cols = cnt
    fig, ax = plt.subplots(1, cnt)    
    if 1 == cnt:
        ax = [ax]
    for i in range(0, len(images)):
        dims = len(images[i].shape)
        if 3 == dims:
            imgDisp = cv.cvtColor(images[i], cv.COLOR_BGR2RGB)
        else:
            imgDisp = cv.cvtColor(images[i], cv.COLOR_GRAY2RGB)
            
        ax[i].axis('off')
        ax[i].imshow(imgDisp)
        
def Distance(pt1, pt2):
    dx = float(pt2[0])-float(pt1[0])
    dy = float(pt2[1])-float(pt1[1])
    return math.sqrt(dx*dx + dy*dy)
        
def ColorRange(clr, fuzz):    
    colorMin = (max(0,clr[0]-fuzz), max(0,clr[1]-fuzz), max(0,clr[2]-fuzz))    
    colorMax = (min(255,clr[0]+fuzz), min(255,clr[1]+fuzz), min(255,clr[2]+fuzz))
    return colorMin, colorMax
        
#Handy color switcher
clrs = []
clrs.append((255,0,0))
clrs.append((0,255,0))
clrs.append((0,0,255))
clrs.append((255,255,0))
clrs.append((255,0,255))
clrs.append((0,255,255))
iClr = 0

def Color():
    global iClr
    iClr += 1
    return clrs[iClr % len(clrs)]
    
clr = Color()




def LoadMarkers(subDir):
    files, dir = FindImageFilesAndDir(subdir=subDir, suffixFilter="py")
    filenameMarkersPy = "{0}/markers.py".format(dir)
    print("Loading Markers from: {0}".format(filenameMarkersPy))
    #importlib.import_module(filenameMarkersPy) 
    spec = importlib.util.spec_from_file_location("markers", filenameMarkersPy)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    markers = module.markers
    return markers
    


def GenerateCalibration(dirCheckerboard, rotate=False):
    global calRotate
    calRotate = rotate
    files, dir = FindImageFilesAndDir(subdir=dirCheckerboard)
    print("Generating Calibration")
    print(" rotate={0}".format(rotate))
    print(" dir={0}".format(dirCheckerboard))
    print(" Checkerboard file count={0}".format(len(files)))
    patternSize = (9,6)
    patW = patternSize[1]
    patH = patternSize[0]

    # Create checkerboard points in 3D space
    objp = np.zeros((patW*patH,3), np.float32)
    objp[:,:2] = np.mgrid[0:patH,0:patW].T.reshape(-1,2)

    # Sub-Pixel setup
    winSize = (5, 5)
    zeroZone = (-1, -1)
    criteria = (cv.TERM_CRITERIA_EPS + cv.TermCriteria_COUNT, 30, 0.001)

    # Go find the pattern in each image
    objpoints = [] # 3d point in real world space
    imgpoints = [] # 2d points in image plane.
    for file in files:
        image = cv.imread(file)
        #print("cal file: {0} shape={1}".format(file, image.shape))
        if(calRotate):
            image = cv.rotate(image,cv.ROTATE_90_CLOCKWISE)
        imageGray = cv.cvtColor(image, cv.COLOR_BGR2GRAY)
        bFound, cornerPoints = cv.findChessboardCorners(imageGray, patternSize)
        if not bFound:
            continue
        objpoints.append(objp) # All the same
        cornerPointsSubPix = cv.cornerSubPix(imageGray, cornerPoints, winSize, zeroZone, criteria)
        imgpoints.append(cornerPoints)
        cv.drawChessboardCorners(image, patternSize, cornerPointsSubPix, bFound)
        
    global calMatrix  
    global calDist
    ret, calMatrix, calDist, rvecs, tvecs = cv.calibrateCamera(objpoints, imgpoints, imageGray.shape[::-1], None, None)
        
    h, w = image.shape[:2]        
    global calMatrixOptimal
    global calROI
    calMatrixOptimal, calROI = cv.getOptimalNewCameraMatrix(calMatrix, calDist, (w,h), 1, (w,h))  


def GenerateCalibrationWithMarkers(dirCheckerboard, rotate=True):
    print("GenerateCalibrationWithMarkers")
    GenerateCalibration(dirCheckerboard=dirCheckerboard,rotate=rotate)
    
    markers = LoadMarkers(dirCheckerboard)
    print("markers=", markers)
    
    # Swap axis?
    if rotate:
        for i in range(0, len(markers)):
            x,y = markers[i]
            markers[i] = (y,x)
    return markers
    
def CalibrateImage(img):    
    # undistort and crop the image
    global calMatrix  
    global calDist
    global calMatrixOptimal
    #global calRotate
    if(calRotate):
        img = cv.rotate(img,cv.ROTATE_90_CLOCKWISE)
    imgUndistort = cv.undistort(img, calMatrix, calDist, None, calMatrixOptimal)
    x, y, w, h = calROI
    #imgUndistort = imgUndistort[y:y+h, x:x+w]
    return imgUndistort

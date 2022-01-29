import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
import math
import importlib.util
import os
import pickle

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

def ImgShowInternal(images, resolution=120):
    plt.rcParams['figure.dpi'] = resolution
    
    # If a single image is provided, turn it into an array of 1
    if "<class 'numpy.ndarray'>" == type(images):
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
        
def ImgShow(images, resolution=120, cols=0):
    if 0 == cols:
        ImgShowInternal(images, resolution)
        return
    
    while len(images) > 0:
        grpCnt = min(len(images), cols)
        subset = images[0:grpCnt]
        ImgShowInternal(subset, resolution)
        images = images[grpCnt:len(images)]

        
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
    
def FindAllChessboardCorners(img):
    patH = 9
    patW = 6
    patternSize = (patH, patW)
    
    imageMasked = img.copy()
    #imageMasked = cv.cvtColor(img, cv.COLOR_BGR2GRAY)

    # Create checkerboard points in 3D space
    objp = np.zeros((patW*patH,3), np.float32)
    objp[:,:2] = np.mgrid[0:patH,0:patW].T.reshape(-1,2)
    
    cornerSets = []
    while True:
        # Find one chessboard array
        bFound, cornerPoints = cv.findChessboardCorners(imageMasked, patternSize,flags=cv.CALIB_CB_FAST_CHECK)
        if not bFound:
            break
        
        # Extract the points on the four corners of it
        pts = []
        indexes = [0, patH-1, len(cornerPoints)-1, len(cornerPoints)-patH]
        for idx in indexes:
            p = cornerPoints[idx][0]
            pts.append([int(round(p[0])), int(round(p[1]))])
        pts = np.array(pts)
        
        # Destroy that checkerboard by drawing a polygon on top of it
        cv.fillPoly(imageMasked, [pts], (255,0,0))
        
        cornerSets.append(cornerPoints)
    return cornerSets

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
        if calRotate:
            image = cv.rotate(image,cv.ROTATE_90_CLOCKWISE)
        imageGray = cv.cvtColor(image, cv.COLOR_BGR2GRAY)
        cornerSets = FindAllChessboardCorners(image)
        if 0==len(cornerSets):
            continue
        
        for cornerPoints in cornerSets:
            imgpoints.append(cornerPoints)
            objpoints.append(objp) # All the same
        
    print(f' Checkerboard count={len(imgpoints)}')

    global calMatrix  
    global calDist
    ret, calMatrix, calDist, rvecs, tvecs = cv.calibrateCamera(objpoints, imgpoints, imageGray.shape[::-1], None, None)
        
    h, w = image.shape[:2]        
    global calMatrixOptimal
    global calROI
    calMatrixOptimal, calROI = cv.getOptimalNewCameraMatrix(calMatrix, calDist, (w,h), 1, (w,h))  



def PickleCalibration(dirCal, bWrite):    
    global calRotate
    global calMatrix  
    global calDist
    global calMatrixOptimal
    global calROI
    filename = "../test/images/" + dirCal + "/cal.pickle"    
    
    if not bWrite:
        # Reading, does the file exist?
        if not os.path.exists(filename):
            return False        
        
    code = 'wb' if bWrite else 'rb'
    with open(filename, code) as f:
        if bWrite:
            pickle.dump(calRotate,          f, pickle.HIGHEST_PROTOCOL)
            pickle.dump(calMatrix,          f, pickle.HIGHEST_PROTOCOL)
            pickle.dump(calDist,            f, pickle.HIGHEST_PROTOCOL)
            pickle.dump(calMatrixOptimal,   f, pickle.HIGHEST_PROTOCOL)
            pickle.dump(calROI,             f, pickle.HIGHEST_PROTOCOL)
        else:
            calRotate = pickle.load(f)
            calMatrix  = pickle.load(f) 
            calDist = pickle.load(f)
            calMatrixOptimal = pickle.load(f)
            calROI = pickle.load(f)
            
    return True

def LoadCalibration(dirCal):
    if PickleCalibration(dirCal, False):
        return  # The cal data was found and loaded
    
    # No cal data on disk, generate it
    GenerateCalibration(dirCal, False)
    
    # And save it in the directory
    PickleCalibration(dirCal, True)
    
def GetCalibrationROI():
    return calROI

def SetCalibrationROI(roiNew):
    global calROI
    print(f'Adjusting ROI: {calROI} -> {roiNew}')
    calROI = roiNew
    
def CalibrationExists():
    return 'calMatrix' in globals()
    
def CalibrateImage(img):    
    # undistort and crop the image
    global calMatrix  
    global calDist
    global calMatrixOptimal
    global calRotate
    if calRotate:
        img = cv.rotate(img,cv.ROTATE_90_CLOCKWISE)
    img = cv.undistort(img, calMatrix, calDist, None, calMatrixOptimal)
    x, y, w, h = calROI
    imgClipped = img[y:y+h, x:x+w]
    return imgClipped, img


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




def TX_Generate(ptsSrc):
    print('TX_Generate')
    if type(ptsSrc) != 'numpy.ndarray':
        ptsSrc = np.array(ptsSrc, np.float32)    

    # The transformed space should be in dimensions that have the 
    # correct aspect ratio for a pool table (2:1). For now, we will
    # assume an 8' pool table. The play area is 44" x 88", but add
    # some resolution. Let's go with 100ths of an inch.
    global txTargetDims    
    txTargetDims = np.array([44, 88], np.int32)
    txTargetDims = txTargetDims * 100     # 100ths of an inch
    print(f' Target Dims = {txTargetDims}')
        
    ptsDst = np.array([
                    [0,0],
                    [txTargetDims[0]-1, 0],
                    [txTargetDims[0]-1, txTargetDims[1]-1],
                    [0, txTargetDims[1]-1]
                    ] , np.float32)
    
    # Calculate the transform matrix
    global matTxToFlat
    global matTxFromFlat
    matTxToFlat = cv.getPerspectiveTransform(ptsSrc, ptsDst)
    matTxFromFlat = cv.getPerspectiveTransform(ptsDst, ptsSrc)

def TX_GetMats():    
    global matTxToFlat
    global matTxFromFlat
    return matTxToFlat, matTxFromFlat

def TX_GetTargetDims():
    global txTargetDims    
    return txTargetDims

def TX_TxPoint(matTx, pt):
    c3 = matTx[2,0]*pt[0] + matTx[2,1]*pt[1] + matTx[2,2]
    m = [(matTx[0,0]*pt[0] + matTx[0,1]*pt[1] + matTx[0,2])/c3,
         (matTx[1,0]*pt[0] + matTx[1,1]*pt[1] + matTx[1,2])/c3]
    return m

def TX_TxPoints(matTx, pts):
    out = []
    for pt in pts:
        a = TX_TxPoint(matTx, pt)
        out.append(a)
    return out

def TX_ToFlatPoint(pt):
    global matTxToFlat
    return TX_TxPoint(matTxToFlat, pt)

def TX_ToFlatPoints(pts):
    global matTxToFlat
    return TX_TxPoints(matTxToFlat, pts)


def TX_FromFlatPoint(pt):
    global matTxFromFlat
    return TX_TxPoint(matTxFromFlat, pt)

def TX_FromFlatPoints(pts):
    global matTxFromFlat
    return TX_TxPoints(matTxFromFlat, pts)
    
def TX_ToFlatImage(image):
    # Apply the warp transform
    global txTargetDims
    global matTxToFlat
    imageWarp = cv.warpPerspective(image, matTxToFlat, txTargetDims)
    return imageWarp

def ToNpArray(arr, dtype=np.int32):
    if type(arr) != 'numpy.ndarray':
        arrNp = np.array(arr, dtype=dtype)
    else:
        arrNp = arr.astype(dtype)
    return arrNp

def Distance(pts):
    dx = pts[1][0] - pts[0][0]
    dy = pts[1][1] - pts[0][1]
    sumOfSquares = dx*dx + dy*dy
    return math.sqrt(sumOfSquares)

def TrimBorder(arr, siz=1):
    trimmed = arr[siz:arr.shape[0]-siz, siz:arr.shape[1]-siz]
    return trimmed
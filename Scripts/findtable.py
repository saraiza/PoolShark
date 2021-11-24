import cv2 as cv



def ShowImg(title, img):
    dy = img.shape[0]
    dx = img.shape[1]
    maxshow = 400
    if dy > dx:
        scalefactor = maxshow / img.shape[0] #y
    else:
        scalefactor = maxshow / img.shape[1] #x
    
    dx = int(scalefactor * img.shape[1])
    dy = int(scalefactor * img.shape[0])
    siz = dx,dy
    #imgsm = cv.resize(img,dsize=(dx,dy),fx=scalefactor, fy=scalefactor, interpolation = cv.INTER_AREA)
    imgsm = cv.resize(img,dsize=siz, fx=scalefactor, fy=scalefactor, interpolation = cv.INTER_AREA)
    cv.imshow(title, imgsm)

img = cv.imread('C:/dev/PoolShark.a/test/images/IMG_4430.jpg')


ShowImg('orig', img)

cv.waitKey(0) 
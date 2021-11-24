import cv2 as cv



def ShowImg(title, img):
    dy = img.shape[0]
    dx = img.shape[1]
    maxshow = 800
    if dy > dx:
        scalefactor = maxshow / img.shape[0] #y
    else:
        scalefactor = maxshow / img.shape[1] #x
    
    dx = int(scalefactor * img.shape[1])
    dy = int(scalefactor * img.shape[0])
    siz = dx,dy
    imgsm = cv.resize(img,dsize=siz, fx=scalefactor, fy=scalefactor, interpolation = cv.INTER_AREA)
    cv.imshow(title, imgsm)

img = cv.imread('C:/dev/PoolShark.a/test/images/IMG_4430.jpg')
ShowImg('orig', img)

blur = cv.GaussianBlur(img, (5,5), cv.BORDER_DEFAULT)
ShowImg('blur',blur)

canny = cv.Canny(img, 100, 175)
ShowImg('canny',canny)

cv.waitKey(0) 
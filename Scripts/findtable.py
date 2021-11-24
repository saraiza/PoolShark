import cv2 as cv

def ShowImg(title, img):
     cv.imshow(title, img)

img = cv.imread('C:/dev/PoolShark.a/test/images/IMG_4430.jpg')
#cv.imshow('orig', img)
ShowImg('orig', img)

cv.waitKey(0)
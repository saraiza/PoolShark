import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
import math

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
        
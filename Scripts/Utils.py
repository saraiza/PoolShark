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
        
        
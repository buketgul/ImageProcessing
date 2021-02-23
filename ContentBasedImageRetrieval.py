# -*- coding: utf-8 -*-
"""
Created on Sat Dec 19 17:10:31 2020

@author: Buket GÜL
"""
import cv2
import numpy as np
from glob import glob
from operator import itemgetter
import math

def findHist(filename):
    img = cv2.imread(filename)
    row = img.shape[0]
    col = img.shape[1]
    size = row * col
    histR = np.zeros((256))
    histG = np.zeros((256))
    histB = np.zeros((256))
    histH = np.zeros((360))
    
    for i in range(0, row):
        for j in range(0, col):
            # hue
            R = float(img[i, j, 2])
            G = float(img[i, j, 1])
            B = float(img[i, j, 0])
            payda = np.sqrt(((R - G) * (R - G)) + ((R - B) * (G - B)))
            if(payda == 0):
                hue = 0
            else:
                if (G >= B):
                    hue = np.arccos((1 / 2 * ((R - G) + (R - B)) / payda))
                else:
                    hue = 2 * math.pi - np.arccos((1 / 2 * ((R - G) + (R - B)) / payda))
                hue = hue * 180 / math.pi
            hue = int(hue) % 360

            histH[int(hue)] += 1
            histR[int(R)] += 1
            histG[int(G)] += 1
            histB[int(B)] += 1
    
    for i in range(0, 256):
        histR[i] = histR[i] / size
        histG[i] = histG[i] / size
        histB[i] = histB[i] / size
        
    for i in range(0, 360):
        histH[i] = histH[i] / size
        
    hist = np.concatenate((histR, histG, histB, histH), axis = 0)
    return hist

def findAllHistograms():
    filename = input("Please enter the path of the images to be used in training. (Ex. : C:\\Users\\UserName\\Desktop\\Database\\)\nPath : ")
    histogramDest = input("Please enter the folder path where you want to save the histograms. (Ex. : C:\\Users\\UserName\\Desktop\\Data\\Histograms\\)\nPath : ")
    for image in glob(filename + "*"):
        x = image.split("\\")
        x = x[-1].split(".")
        np.save(histogramDest + x[0] + ".npy", findHist(image))

def calcDistance(source, imageFile, isRGB):

    images = {}
    hist = findHist(imageFile)
    if(isRGB):
        a = 0
        b = 768
    else:
        a = 768
        b = 1128

    for filename in glob(source + "*"):
        with open(filename, "rb") as fp:
            histogram = np.load(fp)
            uzaklik = 0.0
            for i in range(a, b):
                uzaklik = uzaklik + (float(histogram[i]) - float(hist[i]))**2
            uzaklik = math.sqrt(uzaklik)
            images[filename] = uzaklik
    benzerImg = sorted(images.items(), key = itemgetter(1))
    return benzerImg

def findSimilar(isRGB):
    image = input("Please enter the path of the image you want to find a similar. (Ex. : C:\\Users\\UserName\\Desktop\\Test\\Image1.jpg)\nPath : ")
    source = input("Please enter the path where you saved the histograms. (Ex. : C:\\Users\\UserName\\Desktop\\Data\\Histograms\\)\nPath : ")
    orgImg = cv2.imread(image)
    cv2.imshow("Original image", orgImg)
    benzerImg = calcDistance(source, image, isRGB)
    allImages = input("Please enter the path of all images(for training). : ")
    print("Original image : ", image, "\n")
    j = 0
    print("Similar Images ;\n")
    for i in benzerImg[0:5]:
        j += 1
        print(j, " : ", i , "\n")
        x = i[0].split("\\")
        x = x[-1].split(".")
        similar = cv2.imread(allImages + x[0] + ".jpg")
        cv2.imshow( "Similar image" + str(j) , similar)
    print("To continue, please close the pictures shown.")
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def calcSuccess():

    testSource = input("Please enter the path of test images. : ")
    source = input("Please enter the path where you saved the histograms. (Ex. : C:\\Users\\UserName\\Desktop\\Data\\Histograms\\)\nPath : ")
    for image in glob(testSource + "*"):
        print("Original image : ", image, "\n")
        RGBSimilar = calcDistance(source, image, 1)
        hueSimilar = calcDistance(source, image, 0)
        j = 0
        print("Similar Images Hue ;\n")
        for i in hueSimilar[0:5]:
            j += 1
            print(j, " : ", i , "\n")
        j = 0
        print("Similar Images RGB ;\n")
        for i in RGBSimilar[0:5]:
            j += 1
            print(j, " : ", i , "\n")
    

def printMenu():

	print("\n\n<Welcome to Content Based Image Retrieval Project>\n")
	print("Opeations :\n")
	print("1 - Find the 5 most similar images to the given image(For RGB).\n")
	print("2 - Find the 5 most similar images to the given image(For HSV).\n")
	print("3 - Training.\n")
	print("4 - Calculate succes.\n")
	print("0 - Quit.\n")

def main():

    select = 1
    while(int(select)):
        printMenu()
        select = input("Please select an operation\n\t> ")
        if(int(select) == 1):                                    # RGB
            findSimilar(1)
        elif(int(select) == 2):                                  # HSV
            findSimilar(0)
        elif(int(select) == 3):                                  # Training
            findAllHistograms()
        elif(int(select) == 4):                                  # Calculate Succes Rate
            calcSuccess()
        elif(int(select) == 0):
            select = 0
        else:
            print("Please enter a valid operation.\n")

if __name__ == '__main__':
    main()
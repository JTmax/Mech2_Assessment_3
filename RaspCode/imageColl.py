import os
import urllib.request

f = open("images.txt", "r")

imgfolder = os.path.join(os.getcwd(),"img")




lineList = list()

with open("images.txt") as f:
    for line in f:
        lineList.append(line)
        
print(len(lineList))



x =0
for url in lineList:
    try:
        imgName = os.path.join(imgfolder, str(x) +".jpg")
        print(imgName)
        urllib.request.urlretrieve(url,imgName)
        x = x+1
    except:
        print("error")


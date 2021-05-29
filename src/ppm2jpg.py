import cv2

from PIL import Image, ImageFile
ImageFile.LOAD_TRUNCATED_IMAGES = True


im = Image.open("in_img-0.ppm")
im.save("in_img-0.jpg")
img = cv2.imread("in_img-0.jpg")
crop_img = img[0:359, 1:426]
cv2.imwrite("in_img-0.jpg",crop_img)

im = Image.open("in_img-1.ppm")
im.save("in_img-1.jpg")
img = cv2.imread("in_img-1.jpg")
crop_img = img[0:359, 1:426]
cv2.imwrite("in_img-1.jpg",crop_img)

im = Image.open("in_img-2.ppm")
im.save("in_img-2.jpg")
img = cv2.imread("in_img-2.jpg")
crop_img = img[0:359, 1:426]
cv2.imwrite("in_img-2.jpg",crop_img)

im = Image.open("in_img-3.ppm")
im.save("in_img-3.jpg")
img = cv2.imread("in_img-3.jpg")
crop_img = img[0:359, 1:426]
cv2.imwrite("in_img-3.jpg",crop_img)

im = Image.open("in_img-4.ppm")
im.save("in_img-4.jpg")
img = cv2.imread("in_img-4.jpg")
crop_img = img[0:359, 1:426]
cv2.imwrite("in_img-4.jpg",crop_img)

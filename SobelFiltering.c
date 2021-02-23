#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LINE_SIZE	255

typedef struct{
	char PGMInfo[2];													// Resmin baþýndaki bilgi.
	int width, height, max;												// Satýr, sütun ve maximum deðer.
	unsigned char *mat;													// Pixeller
}IMG;

IMG readingOrgImage(char *fileName);									// Resim dosyasýný okur
void sobelFiltering(IMG orgImg);										// Sobel filtesi uygular
void writingImg(int *img, const char *fileName, IMG orgImg);			// Oluþturulan resimleri yazar.

int main(void) {
	
	char fileName[32];
	IMG orgImg;
	int *img;
	int i, j;
	printf("Please enter the pgm file name.\n");
	scanf("%s", fileName);
	orgImg = readingOrgImage(fileName);
	sobelFiltering(orgImg);
	return 0;
}

IMG readingOrgImage(char *fileName){
	
	IMG orgImg;											//Orjinal resim
	FILE *pgm;
	int i, j;
	char line[MAX_LINE_SIZE];
	pgm = fopen(fileName, "rb");
	if(pgm == NULL){
		printf("Err : Can NOT open pgm file.");
		exit(-1);
	}
	
	fgets(orgImg.PGMInfo, MAX_LINE_SIZE, pgm);			// PGM resmin türü alýnýr.
	fgets(line, MAX_LINE_SIZE, pgm);					// Varsa yorum satýrý yoksa satýr sütun bilgisi alýnýr.
	while (line[0]=='#' || line[0]=='\n'){				// Olan yorum satýrlarý alýnýr.
		fgets(line, MAX_LINE_SIZE, pgm);
	}
    sscanf (line,"%ld %ld", &orgImg.width, &orgImg.height);
	fscanf(pgm, "%d", &orgImg.max);

	orgImg.mat = (unsigned char *)malloc(orgImg.height * orgImg.width * sizeof(unsigned char ));
	if(orgImg.mat == NULL){
		printf("Err : Can NOT allocate memory for matrix.\n");
		exit(-2);
	}
	fread(orgImg.mat, sizeof(unsigned char), orgImg.height * orgImg.width, pgm);	// Resim bilgisi diziye okunur.
	return orgImg;
}


void sobelFiltering(IMG orgImg){
	
	int *imgX;							// Gradient X
	int *imgY;							// Gradient Y
	int *img;							// Sobel
	int threshold;
	unsigned int i, j, k, l;
	int tmpPixelX = 0, tmpPixelY = 0;
	int minX = 0, maxX = 0;
	int minY = 0, maxY = 0;
	int min = 0, max = 0;
	const int sobelY[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int sobelX[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    int add[3][3] = {{- 1 - orgImg.width, - orgImg.width, - orgImg.width + 1}, 
					 {- 1, 0, 1}, 
					 {- 1 + orgImg.width, orgImg.width, + 1 + orgImg.width}};		// Filtre uygulanan pikselin komþu piksellerine eriþmek için oluþturuldu.
	FILE *fx, *fy, *fs;
    
	img = (int *)calloc(orgImg.height * orgImg.width, sizeof(int));
	if(img == NULL){
		printf("Err : Can NOT allocate memory for image.\n");
		exit(-4);
	}
	imgX = (int *)calloc(orgImg.height * orgImg.width, sizeof(int));
	if(imgX == NULL){
		printf("Err : Can NOT allocate memory for image.\n");
		exit(-5);
	}
	imgY = (int *)calloc(orgImg.height * orgImg.width, sizeof(int));
	if(imgY == NULL){
		printf("Err : Can NOT allocate memory for image.\n");
		exit(-6);
	}

	// Minimum ve maximum deðerlerine ilk piksel deðerleri atanýr.
	for(i = 0; i < 3; i++){
		for(j = 0; j < 3; j++){
			tmpPixelX += orgImg.mat[orgImg.width + 1 + add[i][j]] * sobelX[i][j];
			tmpPixelY += orgImg.mat[orgImg.width + 1 + add[i][j]] * sobelY[i][j];
		}
	}
	minX = tmpPixelX;
	minY = tmpPixelY;
	min = (int) (sqrt(tmpPixelX * tmpPixelX + tmpPixelY * tmpPixelY));
	// Dizide matris gibi gezinmek için resmin kenarlarýna dokunmadan yapýlan döngü.
	for(i = orgImg.width + 1; i < (orgImg.height - 2) * orgImg.width + 2; i += orgImg.width){
		for(j = i; j < i + orgImg.width - 2; j++){
			tmpPixelX = 0;
			tmpPixelY = 0;
			// Filtrenin uygulanmasý.
			for(k = 0; k < 3; k++){
				for(l = 0; l < 3; l++){
					tmpPixelX += orgImg.mat[j + add[k][l]] * sobelX[k][l];
					tmpPixelY += orgImg.mat[j + add[k][l]] * sobelY[k][l];
				}
			}
			imgX[j] = tmpPixelX;
			imgY[j] = tmpPixelY;
			img[j] = (int) (sqrt(imgX[j] * imgX[j] + imgY[j] * imgY[j]));
			// Min-max normalizasyonu için minimum maximum deðerlerinin belirlenmesi.
			if(minX > imgX[j]){
				minX = imgX[j];
			}
			if(minY > imgY[j]){
				minY = imgY[j];
			}
			if(maxX < imgX[j]){
				maxX = imgX[j];
			}
			if(maxY < imgY[j]){
				maxY = imgY[j];
			}
			if(min > img[j]){
				min = img[j];
			}
			if(max < img[j]){
				max = img[j];
			}
		}
	}
	printf("Please enter the threshold value for the edge detection.\n");
	scanf("%d", &threshold);
	// Min-max normalizasyonu için oluþturuldu.
	for(i = orgImg.width + 1; i < (orgImg.height - 2) * orgImg.width + 2; i += orgImg.width){
		for(j = i; j < i + orgImg.width - 2; j++){
			imgX[j] = ((imgX[j] - minX) * 255 / (maxX - minX));
			imgY[j] = ((imgY[j] - minY) * 255 / (maxY - minY));
			img[j] = (int) ((img[j] - min) * 255 / (max - min));
			// Belirlenen threshold deðerine göre resim düzenlenir.
			if(img[j] < threshold){
				img[j] = 0;
			}
			else{
				img[j] = 255;
			}
		}
	}
	// Resimlerin yeni dosyaya yazýlmasý.
	writingImg(imgX, "Gx.pgm", orgImg);
	writingImg(imgY, "Gy.pgm", orgImg);
	writingImg(img, "Sobel.pgm", orgImg);
}


void writingImg(int *img, const char *fileName, IMG orgImg){
	
	FILE *f;
	int i;
	
	f = fopen(fileName, "wb");
	if(f == NULL){
		printf("Err : Can NOT open file for gradient x.\n");
		exit(-7);
	}
	// Önce resim bilgileri, daha sonra piksel bilgileri dosyaya yazýlýr.
	fprintf(f, "%s\n", orgImg.PGMInfo);
	fprintf(f, "%d %d\n", orgImg.width, orgImg.height);
	fprintf(f, "%d\n", orgImg.max);
	for(i = 0; i < orgImg.height * orgImg.width; i++){
		fprintf(f, "%c", (unsigned char)img[i]);
	}
	fclose(f);
}


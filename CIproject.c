#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double ActivityToPower(float alpha, double activity[300][278], double audioPwr[300][279], int blkSize);
double ElFieldToActivity(double efdata[300][278],float normoffset[],int nl, float nlExp);

double ActivityToPower(float alpha, double activity[300][278], double audioPwr[300][279], int blkSize) {
	double temp=0;
	//audioPwr[:, k] * alpha + activity[:, k] * (1 - alpha)
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			temp = audioPwr[i][j] * alpha + activity[i][j] * (1.0 - alpha);
			if (activity[i][j] > audioPwr[i][j]) {
				temp = activity[i][j];
			}
			audioPwr[i][j + 1] = temp;
		}
	}
		
	//audioPwr[:,0] = audioPwr[:,blkSize]
	for (int i = 0; i < 300; i++) {
		audioPwr[i][0] = audioPwr[i][blkSize];
	}

	FILE* audioPwr1;
	audioPwr1 = fopen("audioPwr[5282 - 5559]-C.csv", "w+");
	for (int i = 0; i < 300; i++)
	{
		for (int j = 0; j < 279; j++)
		{
			fprintf(audioPwr1, "%f,", audioPwr[i][j]);
		}
		fprintf(audioPwr1, "\n");
	}
	fclose(audioPwr1);

}

double ElFieldToActivity(double efdata[300][278], float normoffset[], int nl, float nlExp) {
	double efData_save[300][278];

	//efData = (efData - normOffset)
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			efData_save[i][j]= efdata[i][j] - normoffset[i];
		}
	}

	//electricField = np.maximum(0,efData)
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			if (0 > efData_save[i][j])
				efData_save[i][j] = 0;
		}
	}

	// electricField = electricField / 0.4 * 0.5
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			efData_save[i][j] = efData_save[i][j] / 0.4 * 0.5;
		}
	}

	//np.exp(nl*electricField)
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			efData_save[i][j] = exp(nl * efData_save[i][j]);
		}
	}

	//np.minimum(np.exp(nl*electricField),nlExp) -1
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			if (efData_save[i][j] > nlExp)
				efData_save[i][j] = nlExp;
			efData_save[i][j] -= 1;
		}
	}

	//np.maximum(0,np.minimum(np.exp(nl*electricField),nlExp)-1)
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			if (0 > efData_save[i][j])
				efData_save[i][j] = 0;
		}
	}

	//np.maximum(0,np.minimum(np.exp(nl*electricField),nlExp)-1)/(nlExp - 1)
	for (int j = 0; j < 278; j++) {
		for (int i = 0; i < 300; i++) {
			efData_save[i][j] /= (nlExp - 1.0);
		}
	}
  
	FILE* fpt;
	fpt = fopen("activity[5282 - 5559]-C.csv", "w+");
	for (int i = 0; i < 300; i++)
	{
		for (int j = 0; j < 278; j++)
		{
			fprintf(fpt, "%f,", efData_save[i][j]);
		}
		fprintf(fpt, "\n");
	}
	fclose(fpt);
}

//Create global variable
double efData[300][278];
double line[1500];
float normOffset[300];
double audioPwr[300][279];
double activity_save[300][278];
double activity_save_p[300][278];

int main() {
	int nl = 5;
	int blkSize = 278;
	int captFs = 55556;
	float nlExp = exp(nl);//148.4131591025766;
	float taus = 0.01;
	float alpha = exp(-1 / (taus * captFs)); // 0.9982016334024256

  //Test the number from 5282-5559
	FILE* csvFile = fopen("efData[5282 - 5559].csv", "r");
	if (!csvFile) {
		printf("Unable to open the file!");
		exit(1);
	}
	
	for (int i = 0; i < 300; i++) {
		if (fgets(line, sizeof(line), csvFile) != NULL) {
			char* token = strtok(line, ","); //Bat dau dong moi
			for (int j = 0; j < 278; j++) {
				efData[i][j] = atof(token);
				if ((token = strtok(NULL, ",")))
					continue;
			}
		}
	}

	fclose(csvFile);

	for (int i = 0; i < 300; i++) {
		normOffset[i] = 0.0909090909090909;
	}


	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 279; j++) {
			audioPwr[i][j]=0;
		}
	}

	ElFieldToActivity(efData,normOffset,nl,nlExp);

	FILE* activityFile = fopen("activity[5282 - 5559]-C.csv", "r");
	if (!activityFile) {
		printf("Unable to open the file!");
		exit(1);
	}

	for (int i = 0; i < 300; i++) {
		if (fgets(line, sizeof(line), activityFile) != NULL) {
			char* token = strtok(line, ","); //Bat dau dong moi
			for (int j = 0; j < 278; j++) {
				activity_save[i][j] = atof(token);
				if ((token = strtok(NULL, ",")))
					continue;
			}
		}
	}

	fclose(activityFile);
	
	ActivityToPower(alpha, activity_save, audioPwr, blkSize);

	//Testing 

	FILE* activityFileP = fopen("activity[5282 - 5559].csv", "r");
	if (!activityFileP) {
		printf("Unable to open the file!");
		exit(1);
	}

	for (int i = 0; i < 300; i++) {
		if (fgets(line, sizeof(line), activityFileP) != NULL) {
			char* token = strtok(line, ","); //Bat dau dong moi
			for (int j = 0; j < 278; j++) {
				activity_save_p[i][j] = atof(token);
				if ((token = strtok(NULL, ",")))
					continue;
			}
		}
	}

	fclose(activityFileP);

	//float sum = 0,sum2=0;
	//for (int i = 0; i < 300; i++) {
	//	for (int j = 0; j < 278; j++) {
	//		sum += activity_save[i][j];		//C
	//		sum2 += activity_save_p[i][j];	//Python
	//	}
	//}
	//printf("%f\n", sum);
	//printf("%f", sum2);

	//for (int i = 0; i < 300; i++) {
	//	for (int j = 0; j < 278; j++) {
	//		if (activity_save[i][j] != activity_save_p[i][j]) {
	//			printf("%f\n", activity_save[i][j]);
	//			printf("%f\n", activity_save_p[i][j]);
	//		}
	//	}
	//	printf("\n");
	//}


	//Check AudioPwr
	FILE* audioPwrFile = fopen("audioPwr[5282 - 5559]-C.csv", "r");
	if (!audioPwrFile) {
		printf("Unable to open the file!");
		exit(1);
	}

	for (int i = 0; i < 300; i++) {
		if (fgets(line, sizeof(line), audioPwrFile) != NULL) {
			char* token = strtok(line, ","); //Bat dau dong moi
			for (int j = 0; j < 279; j++) {
				activity_save[i][j] = atof(token);
				if ((token = strtok(NULL, ",")))
					continue;
			}
		}
	}
	fclose(audioPwrFile);

	FILE* audioPwrFileP = fopen("audioPwr[5282 - 5559].csv", "r");
	if (!audioPwrFileP) {
		printf("Unable to open the file!");
		exit(1);
	}

	for (int i = 0; i < 300; i++) {
		if (fgets(line, sizeof(line), audioPwrFileP) != NULL) {
			char* token = strtok(line, ","); //Bat dau dong moi
			for (int j = 0; j < 279; j++) {
				activity_save_p[i][j] = atof(token);
				if ((token = strtok(NULL, ",")))
					continue;
			}
		}
	}
	fclose(audioPwrFileP);

	float sum = 0, sum2 = 0;
	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 279; j++) {
			sum += activity_save[i][j];		//C
			sum2 += activity_save_p[i][j];	//Python
		}
	}
	printf("%f\n", sum);
	printf("%f", sum2);

	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 279; j++) {
			if (activity_save[i][j] != activity_save_p[i][j]) {
				printf("%f\n", activity_save[i][j]);
				printf("%f\n", activity_save_p[i][j]);
			}
		}
		printf("\n");
	}

	return 0;
}

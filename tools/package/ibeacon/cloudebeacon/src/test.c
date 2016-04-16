#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "base64RSA.h"
#include "cJSON.h"








#if 0
// 
int main()
{
	cJSON *root,*fmt, *human, *vroot;
	char *out;	/* declare a few. */
	
	root = cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
	cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
	cJSON_AddFalseToObject (fmt, "interlace");
	cJSON_AddNumberToObject(fmt, "frame rate", 24);
	//cJSON_AddItemReferenceToObject
	cJSON_AddItemToObject(root, "human", human = cJSON_CreateObject());
	cJSON_AddNullToObject(human, "interlace");
	cJSON_AddNumberToObject(human, "age", 24);
	
	/* Print to text, Delete the cJSON, print it, release the string. */
	out = cJSON_Print(root);	
	printf("%s\n",out);	
	free(out);
	
	vroot = cJSON_CreateObject();
	cJSON_AddItemReferenceToObject(vroot, "name", human);
	out = cJSON_Print(vroot);
	printf("%s\n", out);
	cJSON_Delete(vroot);
	
	cJSON_Delete(root);	
	free(out);
	return 0;
}
#endif

#if 0
// 测试添加json
int main()
{
	char text0[] = "{\n\t\"type\":       \"rect\", \n\t\"width\":      1920, \n\t\"height\":     1080\n}";	

	cJSON *root = cJSON_Parse(text0);
	cJSON_AddStringToObject(root, "mac", "112233445566");
	char* out = cJSON_Print(root);
	cJSON_Delete(root);
	printf("out=%s\n", out);
	free(out);
	return 0;
}
#endif 



#if 0
CBase64Operate base64;
// mac 地址和key规则匹配
int main()
{
	char *mac = "112233445566";
	int i = 0, j;
	
	unsigned char key[17] = "1234567890abcdef";
	for (j=0, i=0; i<16; ++i, ++j)
	{
		key[i] = key[i] + mac[j];
		j = j % 12;
	}
	for (i=0; i<16; ++i)
	{
		printf("%0x", key[i]);
	}
	printf("\n");

	char output[1024] = {0};
	unsigned long len;

	// base64
	base64.Encrypt((char*)key, 16, output, len);
	output[len] = 0;
	printf("output=%s\n", output);

	std::string data;
	base64.Decrypt(output, len, data);
	memcpy(key, data.c_str(), 16);
	for (j=0, i=0; i<16; ++i, ++j)
	{
		printf("%0x", key[i]);
		key[i] = key[i] - mac[j];
		j = j % 12;
	}
	printf("\n");

	printf("%s\n", key);
	
	return 0;
}
#endif 




int
LoadFileToMem(char** buff, const char* FilePath)
{
	struct stat sb;
	if ( -1 == stat(FilePath, &sb) )
	{
		printf("stat file failed.");
		return -1;
	}
	int fileSize = sb.st_size+1;

	*buff = (char*)malloc(fileSize);
	if (NULL == *buff)
	{
		printf("Malloc memory failed.\n");
		return -1;
	}
	memset(*buff, 0, fileSize);

	FILE *fd = fopen(FilePath, "r");
    if (NULL == fd)
    {
        printf("Open %s Failed.\n", FilePath);
		free(*buff), *buff = NULL;
        return -1;
    }

	int nRead = 0;
	int j = fileSize / 512 + 1;
	nRead = fread(*buff, 512, j, fd);
	if (nRead != j-1)
	{
		printf("Error: fread to file failed.nRead=%d, j=%d\n", nRead, j);
		free(*buff), *buff = NULL;
        return -1;
	}
    fclose(fd);
	printf("333333333333333333\n");
	return fileSize;
}


int main()
{
	int nRet;
	char* buff = NULL;
	nRet = LoadFileToMem(&buff, "./abc.dat");
	if (buff)
		printf("buff:%s\n", buff);
	return 0;
}






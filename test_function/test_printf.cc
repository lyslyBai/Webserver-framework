#include <stdio.h>


int main() {
    int a = 266;
    float b = 1.11;
    int* c = &a;
    char d = 'a';
    // char str = 'a';
    char e[] = "abcdf";
    
    /*%x 表示以十六进制整数形式输出。
    %.2x 中的 .2 表示至少输出两位宽度的十六进制数。如果实际的十六进制数不足两位，则在前面填充零。*/
    printf("%d\n",a);
    printf("%p\n",c);
    printf("%.2f\n",b);
    printf("%.32x\n",a);
    printf("%s\n",e);
    printf("%c\n",d);

    return 0;
}
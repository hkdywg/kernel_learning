/*
 * 插入排序 
 *
 * */

#include <stdlib.h>
#include <stdio.h>


void insertsort(int *arr, int len)
{
    for(int j = 1; j < len; j++)
    {
        int i = 0;
        while(arr[i] < arr[j] && i < j)
            i = i + 1;
        int temp = arr[j];
        if(i != j)
        {
            int x = j;
            while(x > i)
            {
                arr[x] = arr[x-1];
                x = x - 1;
            }
            arr[i] = temp;
        }
    }
}

int main(int argc, char *argv[])
{
    int arr[] = {44,3,3,38,5,47,15, 36,26,27,2,46,4,19,50,48};
    int len = sizeof(arr)/sizeof(int);

    insertsort(arr, len);

    for(int i = 0; i < len; i++)
        printf("%d ", arr[i]);
    printf("\n");
    return 0;
}

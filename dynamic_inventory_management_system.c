#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXiMUM_STRING_LENGHT 100

struct productDetail
{ 
    short int productId;
    char productName[MAXiMUM_STRING_LENGHT];
    float productPrice;
    short int productQuantity;
};

struct productDetail* addProduct(struct productDetail *productInstance, short int *totalNumberOfProducts);
void showTotalProduct(struct productDetail *productInstance, short int totalNumberOfProducts);
void updateQuantity(struct productDetail *productInstance, short int totalNumberOfProducts);
void searchByID(struct productDetail *productInstance, short int totalNumberOfProducts);
void searchByName(struct productDetail *productInstance, short int totalNumberOfProducts);
void searchByPriceRange(struct productDetail *productInstance, short int totalNumberOfProducts);
struct productDetail* deleteProduct(struct productDetail *productInstance, short int *totalNumberOfProducts);

int main()
{
    short int totalNumberOfProducts;
    int choice;

    printf("Enter initial number of products: ");
    if (scanf("%d", &totalNumberOfProducts) != 1)
    {
        printf("Error! Invalid Input");
        return 1;
    }
    getchar();

    struct productDetail *productInstance;
    productInstance = (struct productDetail*)calloc(totalNumberOfProducts, sizeof(struct productDetail));

    if (productInstance == NULL)
    {
        printf("Unable to Allocate Memory for Products");
        return 1;
    }

    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        printf("\nEnter details for product %d\n", productIndex + 1);

        printf("Product Id: ");
        if (scanf("%hd", &(productInstance + productIndex)->productId) != 1)
        {
            printf("Error! Invalid Id");
            productIndex--;
            while (getchar() != '\n');
            continue;
        }

        printf("Product Name: ");
        scanf(" %99[^\n]", (productInstance + productIndex)->productName);

        printf("Product Price: ");
        if (scanf("%f", &(productInstance + productIndex)->productPrice) != 1)
        {
            printf("Error! Invalid Product price");
            productIndex--;
            while (getchar() != '\n');
            continue;
        }

        printf("Product Quantity: ");
        if (scanf("%hd", &(productInstance + productIndex)->productQuantity) != 1)
        {
            printf("Error! Invalid Product Quantity");
            productIndex--;
            while (getchar() != '\n');
            continue;
        }
    }

    do
    {
        printf("\n========= INVENTORY MENU =========\n\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity\n");
        printf("4. Search Product by ID\n");
        printf("5. Search Product by Name\n");
        printf("6. Search Product by Price Range\n");
        printf("7. Delete Product\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
            case 1:
                productInstance = addProduct(productInstance, &totalNumberOfProducts);
                break;
            case 2:
                showTotalProduct(productInstance, totalNumberOfProducts);
                break;
            case 3:
                updateQuantity(productInstance, totalNumberOfProducts);
                break;
            case 4:
                searchByID(productInstance, totalNumberOfProducts);
                break;
            case 5:
                searchByName(productInstance, totalNumberOfProducts);
                break;
            case 6:
                searchByPriceRange(productInstance, totalNumberOfProducts);
                break;
            case 7:
                productInstance = deleteProduct(productInstance, &totalNumberOfProducts);
                break;
            case 8:
                printf("Memory released successfully. Exiting program...\n");
                free(productInstance);
                productInstance = NULL;
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (1);
}

void showTotalProduct(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    printf("====== PRODUCT LIST=====\n\n");

    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        printf(" Product ID: %hd |", (productInstance + productIndex)->productId);
        printf(" Name: %s |", (productInstance + productIndex)->productName);
        printf(" Price: %.2f |", (productInstance + productIndex)->productPrice);
        printf(" Quantity: %hd \n", (productInstance + productIndex)->productQuantity);
    }
}

struct productDetail* addProduct(struct productDetail *productInstance, short int *totalNumberOfProducts)
{
    (*totalNumberOfProducts)++;
    productInstance = realloc(productInstance, (*totalNumberOfProducts) * sizeof(struct productDetail));

    if (productInstance == NULL)
    {
        printf("Memory reallocation failed!\n");
        exit(1);
    }

    printf("\nEnter new Product Detail\n");
    printf("Product Id: ");
    scanf("%hd", &(productInstance[*totalNumberOfProducts - 1].productId));
    getchar();

    printf("Product Name: ");
    scanf(" %99[^\n]", productInstance[*totalNumberOfProducts - 1].productName);

    printf("Product Price: ");
    scanf("%f", &(productInstance[*totalNumberOfProducts - 1].productPrice));

    printf("Product Quantity: ");
    scanf("%hd", &(productInstance[*totalNumberOfProducts - 1].productQuantity));

    printf("Product added successfully!\n");

    return productInstance;
}

void searchByID(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    short int searchId;
    int productFound = 0;

    printf("Enter Product ID to search: ");
    if (scanf("%hd", &searchId) != 1)
    {
        printf("Error! Invalid Input\n");
        while (getchar() != '\n');
        return;
    }
    getchar();

    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        if ((productInstance + productIndex)->productId == searchId)
        {
            printf("\nProduct Found:\n");
            printf("Product ID: %hd | Name: %s | Price: %.2f | Quantity: %hd\n",
                   (productInstance + productIndex)->productId,
                   (productInstance + productIndex)->productName,
                   (productInstance + productIndex)->productPrice,
                   (productInstance + productIndex)->productQuantity);
            productFound = 1;
            break;
        }
    }

    if (!productFound)
    {
        printf("Product Not Found.\n");
    }
}

void updateQuantity(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    short int productIdToUpdate;
    int updatedQuantity;
    int productFound = 0;

    printf("Enter Product ID to update quantity: ");
    if (scanf("%hd", &productIdToUpdate) != 1)
    {
        printf("Error! Invalid Product ID\n");
        while (getchar() != '\n');
        return;
    }
    getchar();

    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        if ((productInstance + productIndex)->productId == productIdToUpdate)
        {
            productFound = 1;

            printf("Enter new Quantity: ");
            if (scanf("%hd", &updatedQuantity) != 1)
            {
                printf("Error! Invalid Quantity\n");
                while (getchar() != '\n');
                return;
            }
            getchar();

            (productInstance + productIndex)->productQuantity = updatedQuantity;
            printf("Quantity updated successfully!\n");
            break;
        }
    }

    if (!productFound)
    {
        printf("Product Not Found To Update.\n");
    }
}

void searchByPriceRange(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    float minimumPrice, maximumPrice;
    int productFound = 0;

    printf("Enter minimum price: ");
    if (scanf("%f", &minimumPrice) != 1)
    {
        printf("Error! Invalid Price\n");
        while (getchar() != '\n');
        return;
    }
    getchar();

    printf("Enter maximum price: ");
    if (scanf("%f", &maximumPrice) != 1)
    {
        printf("Error! Invalid Price\n");
        while (getchar() != '\n');
        return;
    }
    getchar();

    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        if ((productInstance + productIndex)->productPrice >= minimumPrice &&
            (productInstance + productIndex)->productPrice <= maximumPrice)
        {
            printf(" Product ID: %hd | Name: %s | Price: %.2f | Quantity: %hd\n",
                   (productInstance + productIndex)->productId,
                   (productInstance + productIndex)->productName,
                   (productInstance + productIndex)->productPrice,
                   (productInstance + productIndex)->productQuantity);
            productFound++;
        }
    }

    if (!productFound)
    {
        printf("Product Not Found in Range %.2f to %.2f.\n", minimumPrice, maximumPrice);
    }
}

void searchByName(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    char searchName[MAXiMUM_STRING_LENGHT];
    int productFound = 0;

    printf("Enter Product Name to search: ");
    scanf(" %99[^\n]", searchName);

    char lowerSearch[MAXiMUM_STRING_LENGHT];
    char *sourcePointer = searchName;
    char *destinationPointer = lowerSearch;

    while (*sourcePointer != '\0')
    {
        *destinationPointer = tolower(*sourcePointer);
        sourcePointer++;
        destinationPointer++;
    }
    *destinationPointer = '\0';

    printf("\nSearch Results:\n");

    for (struct productDetail *currentProduct = productInstance;
         currentProduct < productInstance + totalNumberOfProducts;
         currentProduct++)
    {
        char lowerProductName[MAXiMUM_STRING_LENGHT];
        char *sourceProductPointer = currentProduct->productName;
        char *destinationProductPointer = lowerProductName;

        while (*sourceProductPointer != '\0')
        {
            *destinationProductPointer = tolower(*sourceProductPointer);
            sourceProductPointer++;
            destinationProductPointer++;
        }
        *destinationProductPointer = '\0';

        char *productPointer = lowerProductName;
        int matchFound = 0;

        while (*productPointer != '\0')
        {
            char *temporaryProductPointer = productPointer;
            char *temporarySearchPointer = lowerSearch;

            while (*temporaryProductPointer != '\0' && *temporarySearchPointer != '\0' &&
                   *temporaryProductPointer == *temporarySearchPointer)
            {
                temporaryProductPointer++;
                temporarySearchPointer++;
            }

            if (*temporarySearchPointer == '\0')
            {
                matchFound = 1;
                break;
            }

            productPointer++;
        }

        if (matchFound)
        {
            printf(" Product ID: %hd | Name: %s | Price: %.2f | Quantity: %hd\n",
                   currentProduct->productId,
                   currentProduct->productName,
                   currentProduct->productPrice,
                   currentProduct->productQuantity);
            productFound++;
        }
    }

    if (!productFound)
    {
        printf("Product Not Found for this search.\n");
    }
}

struct productDetail* deleteProduct(struct productDetail *productInstance, short int *totalNumberOfProducts)
{
    if (*totalNumberOfProducts == 0)
    {
        printf("No products available to delete.\n");
        return productInstance;
    }

    short int productIdToDelete;
    int productFound = 0;

    printf("Enter Product ID to delete: ");
    if (scanf("%hd", &productIdToDelete) != 1)
    {
        printf("Error! Invalid Product ID\n");
        while (getchar() != '\n');
        return productInstance;
    }
    getchar();

    for (int productIndex = 0; productIndex < *totalNumberOfProducts; productIndex++)
    {
        if ((productInstance + productIndex)->productId == productIdToDelete)
        {
            productFound = 1;

            for (int shiftIndex = productIndex; shiftIndex < *totalNumberOfProducts - 1; shiftIndex++)
            {
                *(productInstance + shiftIndex) = *(productInstance + shiftIndex + 1);
            }

            (*totalNumberOfProducts)--;

            struct productDetail *temperoryPointer = realloc(productInstance, (*totalNumberOfProducts) * sizeof(struct productDetail));
            if (temperoryPointer != NULL || *totalNumberOfProducts == 0)
            {
                productInstance = temperoryPointer;
            }

            printf("Product deleted successfully!");
            break;
        }
    }

    if (!productFound)
    {
        printf("Product Not Found For Delete.\n");
    }

    return productInstance;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXIMUM_STRING_LENGTH 100

struct productDetail
{ 
    short int productId;
    char productName[MAXIMUM_STRING_LENGTH];
    float productPrice;
    short int productQuantity;
};

int validateIntegerInput(char *inputString);
int validateFloatInput(char *inputString);
short int getValidatedShortInt();
float getValidatedFloat();
void getValidatedString(char *destinationString, int maxLength);

struct productDetail* addProduct(struct productDetail *productInstance, short int *totalNumberOfProducts);
void showTotalProduct(struct productDetail *productInstance, short int totalNumberOfProducts);
void updateQuantity(struct productDetail *productInstance, short int totalNumberOfProducts);
void searchByID(struct productDetail *productInstance, short int totalNumberOfProducts);
void searchByName(struct productDetail *productInstance, short int totalNumberOfProducts);
void searchByPriceRange(struct productDetail *productInstance, short int totalNumberOfProducts);
struct productDetail* deleteProduct(struct productDetail *productInstance, short int *totalNumberOfProducts);
int isDuplicateId(struct productDetail *productInstance, short int totalNumberOfProducts, short int id);

int main()
{
    short int totalNumberOfProducts;
    int choice;

    printf("Enter initial number of products: ");
    totalNumberOfProducts = getValidatedShortInt();

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
        short int productIdInput = getValidatedShortInt();

        if (isDuplicateId(productInstance, productIndex, productIdInput))
        {
            printf("Error! Duplicate Product ID. Please enter a unique ID.\n");
            productIndex--;
            continue;
        }

        (productInstance + productIndex)->productId = productIdInput;

        printf("Product Name: ");
        getValidatedString((productInstance + productIndex)->productName, MAXIMUM_STRING_LENGTH);

        printf("Product Price: ");
        (productInstance + productIndex)->productPrice = getValidatedFloat();

        printf("Product Quantity: ");
        (productInstance + productIndex)->productQuantity = getValidatedShortInt();
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

        choice = getValidatedShortInt();

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
    }
    while (1);
}

void showTotalProduct(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    printf("====== PRODUCT LIST ======\n\n");

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
    short int newProductId = getValidatedShortInt();

    if (isDuplicateId(productInstance, *totalNumberOfProducts - 1, newProductId))
    {
        printf("Error! Duplicate Product ID. Product not added.\n");
        (*totalNumberOfProducts)--;
        return productInstance;
    }

    productInstance[*totalNumberOfProducts - 1].productId = newProductId;

    printf("Product Name: ");
    getValidatedString(productInstance[*totalNumberOfProducts - 1].productName, MAXIMUM_STRING_LENGTH);

    printf("Product Price: ");
    productInstance[*totalNumberOfProducts - 1].productPrice = getValidatedFloat();

    printf("Product Quantity: ");
    productInstance[*totalNumberOfProducts - 1].productQuantity = getValidatedShortInt();

    printf("Product added successfully!\n");

    return productInstance;
}

void updateQuantity(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    short int productIdToUpdate;
    int updatedQuantity;
    int productFound = 0;

    printf("Enter Product ID to update quantity: ");
    productIdToUpdate = getValidatedShortInt();

    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        if ((productInstance + productIndex)->productId == productIdToUpdate)
        {
            productFound = 1;
            printf("Enter new Quantity: ");
            updatedQuantity = getValidatedShortInt();
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

void searchByID(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    short int searchId;
    int productFound = 0;

    printf("Enter Product ID to search: ");
    searchId = getValidatedShortInt();

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

void searchByPriceRange(struct productDetail *productInstance, short int totalNumberOfProducts)
{
    float minimumPrice;
    float maximumPrice;
    int productFound = 0;

    printf("Enter minimum price: ");
    minimumPrice = getValidatedFloat();

    printf("Enter maximum price: ");
    maximumPrice = getValidatedFloat();

    if(minimumPrice >= maximumPrice)
    {
        printf("Error! Minimum Price Must be Greater than Maximum Price");
        return;
    }

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
    char searchName[MAXIMUM_STRING_LENGTH];
    int productFound = 0;

    printf("Enter Product Name to search: ");
    getValidatedString(searchName, MAXIMUM_STRING_LENGTH);

    char lowerSearch[MAXIMUM_STRING_LENGTH];
    int searchCharIndex = 0;

    while (searchName[searchCharIndex] != '\0')
    {
        lowerSearch[searchCharIndex] = tolower(searchName[searchCharIndex]);
        searchCharIndex++;
    }

    lowerSearch[searchCharIndex] = '\0';

    printf("\nSearch Results:\n");

    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        char lowerProductName[MAXIMUM_STRING_LENGTH];
        int charPosition = 0;

        while ((productInstance + productIndex)->productName[charPosition] != '\0')
        {
            lowerProductName[charPosition] = tolower((productInstance + productIndex)->productName[charPosition]);
            charPosition++;
        }

        lowerProductName[charPosition] = '\0';

        int mainIndex = 0;
        int matchFound = 0;

        while (lowerProductName[mainIndex] != '\0')
        {
            int subIndex = 0;

            while (lowerProductName[mainIndex + subIndex] != '\0' &&
                   lowerSearch[subIndex] != '\0' &&
                   lowerProductName[mainIndex + subIndex] == lowerSearch[subIndex])
            {
                subIndex++;
            }

            if (lowerSearch[subIndex] == '\0')
            {
                matchFound = 1;
                break;
            }

            mainIndex++;
        }

        if (matchFound)
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
    productIdToDelete = getValidatedShortInt();

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

            struct productDetail *temporaryPointer = realloc(productInstance, (*totalNumberOfProducts) * sizeof(struct productDetail));

            if (temporaryPointer != NULL || *totalNumberOfProducts == 0)
            {
                productInstance = temporaryPointer;
            }

            printf("Product deleted successfully!\n");
            break;
        }
    }

    if (!productFound)
    {
        printf("Product Not Found For Delete.\n");
    }

    return productInstance;
}

int isDuplicateId(struct productDetail *productInstance, short int totalNumberOfProducts, short int id)
{
    for (int productIndex = 0; productIndex < totalNumberOfProducts; productIndex++)
    {
        if ((productInstance + productIndex)->productId == id)
        {
            return 1;
        }
    }

    return 0;
}

int validateIntegerInput(char *inputString)
{
    int charIndex = 0;

    if (inputString[0] == '\0' || inputString[0] == '-' || inputString[0] == '+')
    {
        return 0;
    }

    while (inputString[charIndex] != '\0')
    {
        if (inputString[charIndex] < '0' || inputString[charIndex] > '9')
        {
            return 0;
        }

        charIndex++;
    }

    return 1;
}

int validateFloatInput(char *inputString)
{
    int charIndex = 0;
    int dotCount = 0;

    if (inputString[0] == '\0' || inputString[0] == '-' || inputString[0] == '+')
    {
        return 0;
    }

    while (inputString[charIndex] != '\0')
    {
        if (inputString[charIndex] == '.')
        {
            dotCount++;

            if (dotCount > 1)
            {
                return 0;
            }
        }
        else if (inputString[charIndex] < '0' || inputString[charIndex] > '9')
        {
            return 0;
        }

        charIndex++;
    }

    return 1;
}

short int getValidatedShortInt()
{
    char inputBuffer[MAXIMUM_STRING_LENGTH];

    while (1)
    {
        fgets(inputBuffer, sizeof(inputBuffer), stdin);

        int index = 0;

        while (inputBuffer[index] != '\n' && inputBuffer[index] != '\0')
        {
            index++;
        }

        inputBuffer[index] = '\0';

        if (validateIntegerInput(inputBuffer))
        {
            return (short int)atoi(inputBuffer);
        }

        printf("Invalid input! Enter a valid integer: ");
    }
}

float getValidatedFloat()
{
    char inputBuffer[MAXIMUM_STRING_LENGTH];

    while (1)
    {
        fgets(inputBuffer, sizeof(inputBuffer), stdin);

        int index = 0;

        while (inputBuffer[index] != '\n' && inputBuffer[index] != '\0')
        {
            index++;
        }

        inputBuffer[index] = '\0';

        if (validateFloatInput(inputBuffer))
        {
            return (float)atof(inputBuffer);
        }

        printf("Invalid input! Enter a valid number: ");
    }
}

void getValidatedString(char *destinationString, int maximumLength)
{
    fgets(destinationString, maximumLength, stdin);

    int index = 0;

    while (destinationString[index] != '\n' && destinationString[index] != '\0')
    {
        index++;
    }

    destinationString[index] = '\0';
}

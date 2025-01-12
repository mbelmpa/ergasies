#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PRODUCTS 20
#define CUSTOMERS 5
#define ORDERS_PER_CUSTOMER 10
// Product struct
struct Product {
 char description[50];//perigrafh proiontos
 float price; //h timh tou
 int item_count; //diathesimo stock proiontwn
 int total_requests; //sunolika aitimata proiontos
 int total_sold; //sunolikes pwlhseis
 char failed_customers[CUSTOMERS * ORDERS_PER_CUSTOMER][10];//lista apo failed pelates
 int failed_count;// failed aitimata
} ;
// catalogos proionton
Product catalog[PRODUCTS];
// arxikopoihsh catalogou
void initialize_catalog() {
 for (int i = 0; i < PRODUCTS; i++) {
   sprintf(catalog[i].description, "Product_%d", i);
   catalog[i].price = (float)(10 + (rand() % 91)); // tixaies times metaksi 10 kai 100
   catalog[i].item_count = 2; // 2 pragmata ana proion
   catalog[i].total_requests = 0;
   catalog[i].total_sold = 0;
   catalog[i].failed_count = 0;
 }
}
// epeksergasia paraggelias
void process_order(int product_id, int customer_id, int customer_order_id, int pipe_fd)
{
 char response[100]; //minima
 catalog[product_id].total_requests++;//aukshsh sinolikwn aitimatwn
 if (catalog[product_id].item_count > 0) //tsekarei an to proion upatxei se apothema
 {
  catalog[product_id].item_count--; //meiwsh tou stock 
  catalog[product_id].total_sold++; //aukshsh arithmou pwlhsewn
  sprintf(response, "Order Success: Product %d | Price: %.2f", product_id, catalog[product_id].price);
 }
 else  //an den iparxei sto apothema tote einai failed order
 {
  sprintf(response, "Order Failed: Product %d is out of stock", product_id);
  sprintf(catalog[product_id].failed_customers[catalog[product_id].failed_count++], "Customer_%d_%d", customer_id, customer_order_id);
 }
 // dinei analogh apanthsh ston pelath
 write(pipe_fd, response, strlen(response) + 1);
 sleep(1); // xronos epeksergasias
}
// ektipwnei sunoptikh anafora olwn twn paraggeliwn
void print_summary() {
 int total_orders = 0;
 int total_success = 0;
 int total_failures = 0;
 float total_revenue = 0;
 printf("\nSummary Report:\n");
 printf("---------------------------------------\n");
 for (int i = 0; i < PRODUCTS; i++) //pernaei ksana apo kathe proion gia na ektipwsei ta statistika
 {
  printf("Product: %s\n", catalog[i].description);
  printf("  Total Requests: %d\n", catalog[i].total_requests);
  printf("  Items Sold: %d\n", catalog[i].total_sold);
  printf("  Failed Customers: ");
  for (int j = 0; j < catalog[i].failed_count; j++)
  {
   printf("%s ", catalog[i].failed_customers[j]);
  }
  printf("\n\n");
//enhmerwnei ta statistika
  total_orders += catalog[i].total_requests;
  total_success += catalog[i].total_sold;
  total_failures += catalog[i].failed_count;
  total_revenue += catalog[i].total_sold * catalog[i].price;
 }
 printf("---------------------------------------\n");
 printf("Total Orders: %d\n", total_orders);
 printf("Total Successful Orders: %d\n", total_success);
 printf("Total Failed Orders: %d\n", total_failures);
 printf("Total Revenue: %.2f\n", total_revenue);
}
int main() {
 int pipes[CUSTOMERS][2][2]; // pipes gia kathe pelath me aithma kai apanthsh
 initialize_catalog();
// dimiourgia pipes kai fork pelatwn
for (int i = 0; i < CUSTOMERS; i++) {
 pipe(pipes[i][0]); // pipe aithmata pelatwn gia agores
 pipe(pipes[i][1]); // pipe gia tis apanthdeis tou katasthmatos se pelates
 pid_t pid = fork();
 if (pid == 0)
 { //process tou pelath
  close(pipes[i][0][0]); 
  close(pipes[i][1][1]); 
  for (int j = 0; j < ORDERS_PER_CUSTOMER; j++)
  {
   int product_id = rand() % PRODUCTS;// tyxaia epilogh proiontos
   write(pipes[i][0][1], &product_id, sizeof(int));
   char response[100]; //dose apanthsh 
   read(pipes[i][1][0], response, sizeof(response));//diavazei apanthsh tou katasthmatos
   printf("Customer %d Order %d: %s\n", i, j + 1, response);
   sleep(1); // anamonh anamesa stis paraggelies
  }
  close(pipes[i][0][1]);
  close(pipes[i][1][0]);
  exit(0);
  }
 }
//epeksergasia E-shop 
 for (int i = 0; i < CUSTOMERS; i++)
 {
  close(pipes[i][0][1]); 
  close(pipes[i][1][0]); 
 }
 for (int i = 0; i < CUSTOMERS * ORDERS_PER_CUSTOMER; i++) //epeksergasia paraggeliwn apo olous tous pelates
 {
  for (int j = 0; j < CUSTOMERS; j++)
  {
   int product_id;
    if (read(pipes[j][0][0], &product_id, sizeof(int)) > 0) //anagnwsh tou id tou proiontos apo to pipe aithmatwn tou pelath
    {
     process_order(product_id, j, i, pipes[j][1][1]);//epeksergasia paraggelias
     }
   }
 }
 for (int i = 0; i < CUSTOMERS; i++)
 {
  wait(NULL); // anamonh gia thn oloklhrwsh ths diadikasias twn pelatwn na oloklirwthei
  close(pipes[i][0][0]); //kleinoume ta ypoleipomena pipes
  close(pipes[i][1][1]);
 }
 print_summary(); //ektupwse thn telikh sunoptikh anafora
 return 0;
}

//this is the code for all my functions 

    #include <stdio.h>
    #include <stdlib.h>
    #include "rand100.h"
    
    unsigned long counter = 0;
    unsigned int ground_size = 0;
    unsigned int num_levels;
    static int prob_percent;
    
   typedef struct dummy_record{
       int key;
       int *data;
       struct dummy_record *nxt; /* points to the next record on its level */
       struct dummy_record *down; /* points to the 'same' record on the level below, ground floor nodes have down = NULL */
   }record;
   
   /* structure that holds pointers to records - used for my stack */
   typedef struct dummy_pointer{
       struct dummy_record *pointing_to; /* points to a record in the superlist */
       struct dummy_pointer *next_pointer; /* points to the next pointer_element in stack */
   }pointer_element;
   
   static pointer_element *stack_head;
   static record *sentinel; /* always points to the top left node */
   
   int init_pointer_stack();
   int push( record *level_pointer );
   record* pop();
   
   int init_pointer_stack()
   {
       stack_head = NULL;
       return 1;
   }
   
   int push( record *level_pointer ) /*pass it a pointer that points to a record within rep*/
   {
       pointer_element *new_pointer;
   
       new_pointer = malloc( sizeof(pointer_element) );
       if( new_pointer == NULL ) return 0;
   
       new_pointer->pointing_to = level_pointer;
       new_pointer->next_pointer = stack_head;
       stack_head = new_pointer;
       return 1;
   }
   
   /* returns a pointer to a record when successful */
   record* pop()
   {
       if( stack_head == NULL ) return NULL;
 
       pointer_element *holding_pointer;
       record *pointing_at_rec; /* returning this record pointer */
   
       pointing_at_rec = stack_head->pointing_to; /* now points to the same record */
       holding_pointer = stack_head;
       stack_head = stack_head->next_pointer;
       free( holding_pointer );
       return pointing_at_rec;
   }
   
   int Repository_init( int p )
   {
       sentinel = malloc(sizeof(record));
       if(sentinel == NULL) return -1;
   
       sentinel->key = 0;
       sentinel->data = NULL;
       sentinel->down = NULL;
       sentinel->nxt = NULL;
   
       prob_percent = p;
       num_levels = 1; /* there will always be at least 1 level */
       return 0;
   }
   
   int Repository_update( int key, int data )
   {
       record *new_rec; /* pointer to the new record we are inserting */
       record *curr_rec; /* pointer to the record we are 'holding' at the moment */
   
       curr_rec = sentinel;
       counter++;
   
       do{
           while ( curr_rec->nxt != NULL && curr_rec->nxt->key <= key ) { /* finds the location on this level */
               if( curr_rec->nxt->key == key ) {
                   *(curr_rec->nxt->data) = data;
                   while ( stack_head != NULL ) pop(); /* frees whatever is left in the stack */
                   return 0;
               }
               curr_rec = curr_rec->nxt;
               counter++;
           }
   
           push(curr_rec); /* saves that levels pointer to our stack */
           curr_rec = curr_rec->down;
          counter++;
  
      } while ( curr_rec != NULL );
  
      /* creates a spot in memory to store data and place the data there */
      int *pointingatdata;
      pointingatdata = malloc(sizeof(data));
      if( pointingatdata == NULL ) return -1;
      *pointingatdata = data;
  do{
          /* adds a new level by creating a new sentinel node */
          if( stack_head == NULL ) { /* stackhead is empty when the new node has inserted on every level */
              record *pnew_sentinel;
              pnew_sentinel = malloc(sizeof(record));
              if( pnew_sentinel == NULL ) return -1;
  
              pnew_sentinel->down = sentinel;
              pnew_sentinel->nxt = NULL;
              pnew_sentinel->key = 0;
              sentinel = pnew_sentinel; /* top left node becomes the new sentinel */
  
              counter++;
              num_levels++;
              push(sentinel);
          }
  
          new_rec = malloc(sizeof(record));
          if( new_rec == NULL ) return -1;
  
          new_rec->key = key;
          new_rec->data = pointingatdata;
          new_rec->down = curr_rec; /* updates nodes down to curr_rec (which now points to the previous insert of new node) */
  
          /* updates adjacent nodes */
          new_rec->nxt = (stack_head->pointing_to)->nxt;
          (stack_head->pointing_to)->nxt = new_rec;
          curr_rec = new_rec; /* note that curr_rec switches now to pointing at the most recent insert of the new node */
          counter+=4;
  
          pop();
  
      } while ( rand100() <= prob_percent );
  
      ground_size++;
      while ( stack_head != NULL ) pop();
      return 1;
  }
  
  int Repository_delete( int key )
  {
      if( sentinel->nxt == NULL ) return 0;
  
      record *curr_rec;
      curr_rec = sentinel;
      counter++;
  
      do{
          while ( curr_rec->nxt != NULL && curr_rec->nxt->key <= key ) { /* searches this level */
              if( curr_rec->nxt->key == key ) { /* we found the key on level, so save a pointer to the prev. record */
                  push(curr_rec);
                  break;
              }
              curr_rec = curr_rec->nxt;
              counter++;
          }
          167         curr_rec = curr_rec->down;
          counter++;
  
      } while ( curr_rec != NULL );
  
      if( stack_head == NULL ) return 0;
  
      /* now our stack holds pointers to all the places we need to delete our node */
      while ( stack_head != NULL ) {
          record *deleting_rec;
          deleting_rec = stack_head->pointing_to->nxt;
  
          record *rec_before_deleting; /* using a temporary pointer because it might be a sentinel node that we have to remove */
          rec_before_deleting = pop(); /* pop() returns a pointer to the record stack_head was pointing at */
  
          if( rec_before_deleting == sentinel && deleting_rec->nxt == NULL && rec_before_deleting->down != NULL ) { /* on the top level and deleting the only node, we must update sentinel */
  
              sentinel = rec_before_deleting->down;
              free(rec_before_deleting);
              num_levels--;
  
          } else if( rec_before_deleting->down != NULL && rec_before_deleting->key == 0 && deleting_rec->nxt == NULL ) { /* not on the ground floor & there is one node on this level */
  
              if( stack_head != NULL ) stack_head->pointing_to->down = rec_before_deleting->down; /* updates level above to point to the correct level */
              free(rec_before_deleting);
              num_levels--;
  
          } else {
  
               rec_before_deleting->nxt = deleting_rec->nxt;
               if( deleting_rec->down == NULL ) free(deleting_rec->data); /* frees the data memory once */
  
          }
  
          free(deleting_rec);
          counter++;
      }
      ground_size--;
      return 1;
  }
  int Repository_get( int key, int *data )
  {
  
      if( sentinel->nxt == NULL ) return 0;
  
      record *curr_rec;
      curr_rec = sentinel;
      counter++;
  
      do{
         while ( curr_rec->nxt != NULL && curr_rec->nxt->key <= key ) {
  
              if( curr_rec->nxt->key == key ) { /* we found the node so we save a copy of its data */
                  *data = *(curr_rec->nxt->data);
                  return 1;
              }
  
              curr_rec = curr_rec->nxt;
              counter++;
          }
 
          curr_rec = curr_rec->down;
          counter++;
 
      } while ( curr_rec != NULL );
  
      return 0;
  }
  
  void Repository_print( int print_elements )
  {
      if( sentinel->nxt == NULL ) return;
  
      int each_level_size[num_levels]; /* arrays element 0 will correlate with level 0 (how many nodes are in each) */
      int i;
  
      for( i=0; i<num_levels; i++ ) each_level_size[i] = 0;
  
      record *curr_rec;
      curr_rec = sentinel;
  
      /* gets to the ground level */
      while ( curr_rec->down != NULL ) curr_rec = curr_rec->down;
      curr_rec = curr_rec->nxt;
  
      while ( curr_rec != NULL )
      {
         record *compare_rec;
          compare_rec = sentinel;
          int on_levels = 1; /* tracks how many times curr_rec appears */
  
          do{
  
              /* finds how many levels curr_rec is on */
              while ( compare_rec->nxt != NULL && compare_rec->nxt->key <= curr_rec->key ) {
  
                  if( compare_rec->nxt->key == curr_rec->key ) {
                      on_levels++;
                      break;
                  }
  
                  compare_rec = compare_rec->nxt;
              }
  
              compare_rec = compare_rec->down;
  
          } while ( compare_rec->down != NULL );
  
          for( i=0; i<on_levels; i++ ) {
  
              if( print_elements == 1 && i==0 ) printf("{%d,%d}\t",curr_rec->key, *(curr_rec->data));
  
              if( print_elements == 2) printf("{%d,%d}\t",curr_rec->key, *(curr_rec->data));
  
              each_level_size[i]++; /* incremenents the amount of nodes on that level */
          }
  
          if( print_elements == 2 || print_elements == 1 ) printf("\n");
          curr_rec = curr_rec->nxt;
      }
  
      printf("Number of levels: %d\n", num_levels);
      printf("Number of unique records: %d\n", ground_size);
      printf("Number of steps performed: %d\n", counter);
  
      for( i=0; i<num_levels; i++ ) printf("Level %d has %d nodes\n",i,each_level_size[i]);
  }
  
                                                                                                                                                                                          297,0-1       Bo

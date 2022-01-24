#include <stdlib.h>
#include <string.h>

#include "cfa.h"
#include "cfa_mem.h"

/* 
AggregationContainers will be added to the existing cfa_conts defined in cfa.c 
*/
extern DynamicArray *cfa_conts;

/* 
create an AggregationContainer within another AggregationContainer 
*/
int 
cfa_def_cont(const int cfa_id, const char* name, int *cfa_cont_idp)
{
    /* check that the cfa_conts has been created */
    if (!cfa_conts)
        return CFA_NOT_FOUND_ERR;

    /* get the aggregation container that we are going to create a new 
    container in */
    AggregationContainer* agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    CFA_CHECK(cfa_err);

    /* Allocate and return the array node (AggregationContainer) */
    AggregationContainer *cont_node = NULL;
    cfa_err = create_array_node(&cfa_conts, (void**)(&cont_node));
    CFA_CHECK(cfa_err);

    /* assign the name, path to NULL */
    cont_node->name = strdup(name),
    cont_node->path = NULL;

    /* set number of vars, dims and containers to 0 */
    cont_node->n_vars = 0;
    cont_node->n_dims = 0;
    cont_node->n_conts = 0;

    /* get the identifier as the last node of the container array */
    int cfa_ncont = 0;
    cfa_err = get_array_length(&cfa_conts, &cfa_ncont);
    CFA_CHECK(cfa_err);
    *cfa_cont_idp = cfa_ncont - 1;

    /* also assign to the parent container */
    agg_cont->cfa_contids[agg_cont->n_conts++] = *cfa_cont_idp;

    return CFA_NOERR;
}

/* 
get the identifier of an AggregationContainer within another 
AggregationContainer, using the name 
*/
int 
cfa_inq_cont_id(const int cfa_id, const char *name, int *cfa_cont_idp)
{

}

/* 
return the number AggregationContainers inside another AggregationContainer
*/
int 
cfa_inq_nconts(const int cfa_id, int *ncontp)
{
    
}

/* 
get the ids for the AggregationContainers in the AggregationContainer
*/
int
cfa_inq_cont_ids(const int cfa_id, int **contids)
{

}

/* 
get the AggregationContainer from a cfa_cont_id 
*/
int 
cfa_get_cont(const int cfa_id, const int cfa_cont_id, 
             AggregationContainer **agg_cont)
{

}

/*
free the memory used by the AggregationContainers and the AggregatedVariables 
and AggregatedDimensions they contain
*/
extern int cfa_free_vars(const int);
extern int cfa_free_dims(const int);
int
cfa_free_cont(const int cfa_id)
{
    /* get the aggregation container struct */
    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    CFA_CHECK(cfa_err);

    /* free the variables */
    cfa_err = cfa_free_vars(cfa_id);
    CFA_CHECK(cfa_err);
    /* free the dimensions */
    cfa_err = cfa_free_dims(cfa_id);
    CFA_CHECK(cfa_err);

    /* free the sub-containers (groups) - recursive call */
    for (int g=0; g<agg_cont->n_conts; g++)
    {
        cfa_err = cfa_free_cont(agg_cont->cfa_contids[g]);
        CFA_CHECK(cfa_err);    
    }

    /* free the path and the name */
    if (agg_cont->path)
    {
        cfa_free(agg_cont->path, strlen(agg_cont->path)+1);
        agg_cont->path = NULL;
    }
    if (agg_cont->name)
    {
        cfa_free(agg_cont->name, strlen(agg_cont->name)+1);
        agg_cont->name = NULL;
    }
    
    /* get the number of none freed array nodes */
    int n_conts = 0;
    cfa_err = get_array_length(&cfa_conts, &n_conts);
    CFA_CHECK(cfa_err);
    int nfc = 0;
    for (int i=0; i<n_conts; i++)
    {
        cfa_err = get_array_node(&cfa_conts, i, (void**)(&(agg_cont)));
        CFA_CHECK(cfa_err);
        /* path == NULL and name == NULL indicates node has been freed */
        if (agg_cont->path || agg_cont->name)
            nfc += 1;
    }
    /* if number of non-free containers is 0 then free the array */
    if (nfc == 0)
    {
        cfa_err = free_array(&cfa_conts);
        cfa_conts = NULL;
    }
    CFA_CHECK(cfa_err);
    return CFA_NOERR;
}
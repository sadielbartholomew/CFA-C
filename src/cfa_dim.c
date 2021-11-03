#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cfa.h"
#include "cfa_mem.h"

/*
create an AggregatedDimension, attach it to a cfa_id
*/
int 
cfa_def_dim(const int cfa_id, const char *name, const int len, int *cfa_dim_idp)
{
    /* get the AggregationContainer */
    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    if (cfa_err)
        return cfa_err;

    /* 
    if no dimensions have been defined previously, then the agg_cont->cfa_dimp 
    will still be NULL
    */
    if (!(agg_cont->cfa_dimp))
    {
        cfa_err = create_array(&(agg_cont->cfa_dimp), 
                    sizeof(AggregatedDimension));
        if (cfa_err)
            return cfa_err;
    }
    /* array is created so now create and return the array node */
    AggregatedDimension *dim_node = NULL;
    cfa_err = create_array_node(&(agg_cont->cfa_dimp), (void**)(&dim_node));
    if (cfa_err)
        return cfa_err;

    /* copy the length and name to the dimension */
    dim_node->len = len;
    dim_node->name = (char*) cfa_malloc(sizeof(char) * strlen(name));
    if (!(dim_node->name))
        return CFA_MEM_ERR;
    strcpy(dim_node->name, name);

    /* get the length of the AggregatedDimension array - the id is len-1 */
    int cfa_ndim = 0;
    cfa_err = get_array_length(&(agg_cont->cfa_dimp), &cfa_ndim);
    if (cfa_err)
        return cfa_err;
    *cfa_dim_idp = cfa_ndim - 1;

    return CFA_NOERR;
}

/*
get the identifier of an AggregatedDimension
*/
int
cfa_inq_dim_id(const int cfa_id, const char* name, int *cfa_dim_idp)
{
    /* get the AggregationContainer */
    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    if (cfa_err)
        return cfa_err;

    /* search through the dimensions, looking for the matching name */
    int cfa_ndim = 0;
    cfa_err = get_array_length(&(agg_cont->cfa_dimp), &cfa_ndim);
    if (cfa_err)
        return cfa_err;

    AggregatedDimension *cdim = NULL;
    for (int i=0; i<cfa_ndim; i++)
    {
        /* dimensions that belong to a closed AggregationContainer have their
        name set to NULL */
        cfa_err = get_array_node(&(agg_cont->cfa_dimp), i, (void**)(&cdim));
        if (cfa_err)
            return cfa_err;
        if (!(cdim->name))
            continue;
        if (strcmp(cdim->name, name) == 0)
        {
            /* found, so assign and return */
            *cfa_dim_idp = i;
            return CFA_NOERR;
        }
    }
    return CFA_DIM_NOT_FOUND_ERR;
}

/*
get the number of AggregatedDimensions defined
*/
int
cfa_inq_ndims(const int cfa_id, int *ndimp)
{
    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    if (cfa_err)
        return cfa_err;
    cfa_err = get_array_length(&(agg_cont->cfa_dimp), ndimp);
    if (cfa_err)
        return cfa_err;
    return CFA_NOERR;
}

/* 
get the AggregatedDimension from a cfa_dim_id
*/
int cfa_get_dim(const int cfa_id, const int cfa_dim_id, 
                AggregatedDimension **agg_dim)
{
#ifdef _DEBUG
    /* check id is in range */
    int cfa_ndims = 0;
    int cfa_err_d = cfa_inq_ndims(cfa_id, &cfa_ndims);
    if (cfa_err_d)
        return cfa_err_d;
    if (cfa_id < 0 || cfa_id >= cfa_ndims)
        return CFA_DIM_NOT_FOUND_ERR;
#endif

    AggregationContainer* agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    if (cfa_err)
        return cfa_err;

    /* 
    check that the path is not NULL.  On cfa_close, the path is set to NULL 
    */
    cfa_err = get_array_node(&(agg_cont->cfa_dimp), cfa_dim_id, 
                             (void**)(agg_dim));
    if (cfa_err)
        return cfa_err;

    if (!(*agg_dim)->name)
        return CFA_DIM_NOT_FOUND_ERR;

    return CFA_NOERR;
}
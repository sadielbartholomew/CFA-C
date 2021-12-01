#include <stdlib.h>
#include <string.h>

#include "cfa.h"
#include "cfa_mem.h"

/* 
create a AggregationVariable container, attach it to a AggregationContainer and one or more AggregatedDimension(s) and assign it to a cfa_var_id
*/
int
cfa_def_var(int cfa_id, const char *name, int ndims, int *cfa_dim_idsp, 
            int *cfa_var_idp)
{
    /* get the aggregation container */
    AggregationContainer* agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    CFA_CHECK(cfa_err);

    /*
    if no variables have been defined previously, then the agg_cont->cfa_varp 
    pointer will be NULL
    */
    if (!(agg_cont->cfa_varp))
    {
        cfa_err = create_array(&(agg_cont->cfa_varp),
                    sizeof(AggregationVariable));
        CFA_CHECK(cfa_err);
    }
    /* Array is create so now allocate and return the array node */
    AggregationVariable *var_node = NULL;
    cfa_err = create_array_node(&(agg_cont->cfa_varp), (void**)(&var_node));
    CFA_CHECK(cfa_err);

    /* assign the name */
    var_node->name = (char*) cfa_malloc(sizeof(char) * strlen(name));
    if (!(var_node->name))
        return CFA_MEM_ERR;
    strcpy(var_node->name, name);

    /* check that the dimension ids have been added to the container already.
       this basically means that the dim_id(s) passed in via the cfa_dim_idsp 
       pointer are greater than 0 and less than the number of dimensions */
    
    int n_cfa_dims = -1;
    cfa_err = cfa_inq_ndims(cfa_id, &n_cfa_dims);
    CFA_CHECK(cfa_err);

    for (int i=0; i<ndims; i++)
    {
        if (cfa_dim_idsp[i] < 0 || cfa_dim_idsp[i] >= n_cfa_dims)
            return CFA_DIM_NOT_FOUND_ERR;
    }

    /* assign the number of dimensions and copy the dimension array */
    var_node->cfa_ndim = ndims;
    var_node->cfa_dim_idp = (int*) cfa_malloc(sizeof(int) * ndims);
    if (!(var_node->cfa_dim_idp))
        return CFA_MEM_ERR;
    memcpy(var_node->cfa_dim_idp, cfa_dim_idsp, sizeof(int) * ndims);

    /* allocate the AggregationVariable identifier */
    int cfa_nvar = 0;
    cfa_err = get_array_length(&(agg_cont->cfa_varp), &cfa_nvar);
    CFA_CHECK(cfa_err);

    /* allocate the AggregationInstructions struct */
    var_node->cfa_instructionsp = cfa_malloc(sizeof(AggregationInstructions));
    var_node->cfa_instructionsp->address = NULL;
    var_node->cfa_instructionsp->location = NULL;
    var_node->cfa_instructionsp->file = NULL;
    var_node->cfa_instructionsp->format = NULL;
    
    /* write back the cfa_var_id */
    *cfa_var_idp = cfa_nvar - 1;

    return CFA_NOERR;
}

/*
get the identifier of an AggregationVariable by name
*/
int
cfa_inq_var_id(const int cfa_id, const char* name, int *cfa_var_idp)
{
    /* get the AggregationContainer with cfa_id */
    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    CFA_CHECK(cfa_err);

    /* search through the variables, looking for the matching name */
    int cfa_nvar = 0;
    cfa_err = get_array_length(&(agg_cont->cfa_varp), &cfa_nvar);
    if (cfa_err)
    {
        if (cfa_err == CFA_MEM_ERR)
            return CFA_VAR_NOT_FOUND_ERR;
        else
            return cfa_err;
    }

    AggregationVariable *cvar = NULL;
    for (int i=0; i<cfa_nvar; i++)
    {
        /* variables that belong to a closed AggregationContainer have their
        name set to NULL */
        cfa_err = get_array_node(&(agg_cont->cfa_varp), i, (void**)(&cvar));
        CFA_CHECK(cfa_err);

        if (!(cvar->name))
            continue;
        if (strcmp(cvar->name, name) == 0)
        {
            /* found, so assign and return */
            *cfa_var_idp = i;
            return CFA_NOERR;
        }
    }
    return CFA_VAR_NOT_FOUND_ERR;
}

/*
get the number of AggregationVariables defined
*/
int 
cfa_inq_nvars(const int cfa_id, int *nvarp)
{
    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    CFA_CHECK(cfa_err);
    if (!(agg_cont->cfa_varp))
    {
        *nvarp = 0;
        return CFA_NOERR;
    }
    cfa_err = get_array_length(&(agg_cont->cfa_varp), nvarp);
    CFA_CHECK(cfa_err);

    return CFA_NOERR;
}

/*
get the AggregationVariable from a cfa_var_id
*/
int
cfa_get_var(const int cfa_id, const int cfa_var_id,
            AggregationVariable **agg_var)
{
#ifdef _DEBUG
    /* check id is in range */
    int cfa_nvars = 0;
    int cfa_err_v = cfa_inq_nvars(cfa_id, &cfa_nvars);
    if (cfa_err_v)
        return cfa_err_v;
    if (cfa_nvars == 0)
        return CFA_VAR_NOT_FOUND_ERR;
    if (cfa_var_id < 0 || cfa_var_id >= cfa_nvars)
        return CFA_VAR_NOT_FOUND_ERR;
#endif

    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    CFA_CHECK(cfa_err);

    /* 
    check that the path is not NULL.  On cfa_close, the path is set to NULL 
    */
    cfa_err = get_array_node(&(agg_cont->cfa_varp), cfa_var_id, 
                             (void**)(agg_var));
    CFA_CHECK(cfa_err);

    if (!(*agg_var)->name)
        return CFA_VAR_NOT_FOUND_ERR;

    return CFA_NOERR;
}

/*
free the memory used by the CFA variables
*/
int
cfa_free_vars(const int cfa_id)
{
    /* get the AggregationContainer */
    AggregationContainer *agg_cont = NULL;
    int cfa_err = cfa_get(cfa_id, &agg_cont);
    CFA_CHECK(cfa_err);
    /* get the number of variables this could be zero if none created yet*/
    int n_vars = 0;
    if (agg_cont->cfa_varp)
    {
        cfa_err = get_array_length(&(agg_cont->cfa_varp), &n_vars);
        CFA_CHECK(cfa_err); 
    }
    else
        return CFA_NOERR;

    /* loop over all the variables and free any associated memory */
    AggregationVariable *agg_var = NULL;
    for (int i=0; i<n_vars; i++)
    {
        cfa_err = get_array_node(&(agg_cont->cfa_varp), i, (void**)(&agg_var));
        CFA_CHECK(cfa_err);
        if (agg_var->name)
        {
            cfa_free(agg_var->name, strlen(agg_var->name));
            agg_var->name = NULL;
        }
        if (agg_var->cfa_dim_idp && agg_var->cfa_ndim > 0)
        {
            cfa_free(agg_var->cfa_dim_idp, sizeof(int) * agg_var->cfa_ndim);
            agg_var->cfa_dim_idp = NULL;
        }
        if (agg_var->cfa_instructionsp)
        {
            /* free the cfa instructions and their strings */
            if (agg_var->cfa_instructionsp->location)
                cfa_free(agg_var->cfa_instructionsp->location,
                         strlen(agg_var->cfa_instructionsp->location) * 
                         sizeof(char));
            if (agg_var->cfa_instructionsp->address)
                cfa_free(agg_var->cfa_instructionsp->address,
                         strlen(agg_var->cfa_instructionsp->address) * 
                         sizeof(char));
            if (agg_var->cfa_instructionsp->file)
                cfa_free(agg_var->cfa_instructionsp->file,
                         strlen(agg_var->cfa_instructionsp->file) * 
                         sizeof(char));
            if (agg_var->cfa_instructionsp->format)
                cfa_free(agg_var->cfa_instructionsp->format,
                         strlen(agg_var->cfa_instructionsp->format) * 
                         sizeof(char));
            cfa_free(agg_var->cfa_instructionsp, 
                     sizeof(AggregationInstructions));
            agg_var->cfa_instructionsp = NULL;
        }
    }
    /* free the array memory */
    cfa_err = free_array(&(agg_cont->cfa_varp));
    CFA_CHECK(cfa_err);
    return CFA_NOERR;
}
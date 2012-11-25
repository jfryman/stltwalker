#include "dbg.h"

#include "matrix.h"
#include "transform.h"

// Object transform containers
stl_transformer *transformer_alloc(stl_object *obj) {
		stl_transformer *t = (stl_transformer*)calloc(1, sizeof(stl_transformer));
		check_mem(t);
		return transformer_init(t, obj);
error:
		exit(-1);
}

void transformer_free(stl_transformer *t) {
		if(t == NULL) return;
		if(t->object) stl_free(t->object);
		free(t);
}

stl_transformer *transformer_init(stl_transformer *t, stl_object *obj) {
		t->object = obj;
		memcpy(t->transform, Identity4x4, sizeof(float4x4));
		return t;
}

void transform_chain(stl_transformer *t, float4x4 transform) {
		float4x4 current;
		memcpy(current, t->transform, sizeof(float4x4));
		mult_4x4f(&t->transform, current, transform);
}

void transform_apply(stl_transformer *t) {
		for(int f = 0; f < t->object->facet_count; f++) {
				stl_facet *facet = t->object->facets + f;
				for(int v = 0; v < 3; v++) {
						float4x1 initial, result;
						float3tofloat4x1(&facet->vertices[v], &initial);
						mult_4x1f(&result, t->transform, initial);
						float4x1tofloat3(&result, &facet->vertices[v]);
				}
		}
		// Reset the transformation matrix
		// so that later invokations don't
		// apply the same transofm we just finished applying
		memcpy(t->transform, Identity4x4, sizeof(float4x4));
}

// Conversion helpers
void float3tofloat4x1(const float3 *v, float4x1 *m) {
		(*m)[0][0] = (*v)[0];
		(*m)[1][0] = (*v)[1];
		(*m)[2][0] = (*v)[2];
		(*m)[3][0] = 1.0f;
}

void float4x1tofloat3(const float4x1 *m, float3 *v) {
		(*v)[0] = (*m)[0][0];
		(*v)[1] = (*m)[1][0];
		(*v)[2] = (*m)[2][0];
}

// Transform matrix initializers
float4x4 *init_transform_scale(float4x4 *t, char *args) {
		float scale = 0.0;
		int rc = sscanf(args, "%f", &scale);
		if(args && strlen(args) > 0) check(rc == 1, "Invalid scale: '%s'", args);
		if(args == NULL || strlen(args) == 0) check(rc == 1, "Scale requires an argument");

		bzero(t, sizeof(float4x4));
		(*t)[0][0] = (*t)[1][1] = (*t)[2][2] = scale;
		(*t)[3][3] = 1.0;

		return t;
error:
		return NULL;
}

// Transformer listing
const transformer transformers[] = {
		{"scale", "Scale the model by a constant factor", init_transform_scale},
		{NULL, NULL, NULL}
};

transform_t transform_find(const char *name) {
		for(transformer *t = (transformer*)transformers; t->name != NULL; t++) {
				if(0 == strncmp(t->name, name,
								strlen(name) < strlen(t->name) ? strlen(name) : strlen(t->name)))
						return t->fun;
		}
		return NULL;
}

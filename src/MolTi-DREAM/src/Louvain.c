/*
    'MolTi' and 'molti-console' detects communities from multiplex networks / 'bonf' computes q-values of annotations enrichment of communities / 'test' simulates random multiplex to test community detection approaches
    Copyright (C) 2015  Gilles DIDIER

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/




#include "Louvain.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define INC_SIZE_BUFFER 10
#define COEFF 0.5
#define EPS 0.0000001
#define THRE log(0.1)
#define RES 1.
#define OFFSET -0.125


/****************************************************************************/
/* Partitions to Graph */
/****************************************************************************/


TypePartition getPartitionConsensus(TypeMultiGraph *graph, TypePartitionMethod type, TypeIterFunc iter, void *info) {
	TypeMultiGraph *gtmp;
	int t, i, j;
	TypePartition *tablePart = (TypePartition*) malloc(graph->sizeTable*sizeof(TypePartition)), res;
	gtmp = (TypeMultiGraph*) malloc(sizeof(TypeMultiGraph));
	gtmp->sizeTable = 1;
	gtmp->edge = (TypeEdgeG***) malloc(sizeof(TypeEdgeG**));
	gtmp->present = (int**) malloc(sizeof(int*));
	gtmp->sizeGraph = graph->sizeGraph;
	gtmp->name = graph->name;
	for(t=0; t<graph->sizeTable; t++) {
		gtmp->edge[0] = graph->edge[t];
		gtmp->present[0] = graph->present[t];
		tablePart[t] = getPartition(gtmp, type,iterateGlobal, info);
	}
	gtmp->present[0] = (int*) malloc(graph->sizeGraph*sizeof(int));
	for(i=0; i<graph->sizeGraph; i++)
		gtmp->present[0][i] = 1;
	gtmp->edge[0] = (TypeEdgeG**) malloc(graph->sizeGraph*sizeof(TypeEdgeG*));
	for(i=0; i<graph->sizeGraph; i++) {
		gtmp->edge[0][i] = (TypeEdgeG*) malloc(graph->sizeGraph*sizeof(TypeEdgeG*));
		gtmp->edge[0][i][i] = 0.;
		for(j=0; j<i; j++) {
			double sum = 0.;
			gtmp->edge[0][i][j] = 0.;
			for(t=0; t<graph->sizeTable; t++) {
				if(graph->present[t][i] && graph->present[t][j]) {
					if(tablePart[t].atom[i] == tablePart[t].atom[j])
						gtmp->edge[0][i][j]++;
					sum++;
				}
			}
			if(sum>0.)
				gtmp->edge[0][i][j] /= sum;
//fprintf(stderr, "e(%d, %d) = %.2lf (%.2lf)\n", i, j, gtmp->edge[0][i][j], graph->edge[0][i][j]);
			gtmp->edge[0][j][i] = gtmp->edge[0][i][j];
		}
	}
	res = getPartition(gtmp, type, iter, info);
	for(i=0; i<graph->sizeGraph; i++)
		free((void*)gtmp->edge[0][i]);
	free((void*)gtmp->edge[0]);
	free((void*)gtmp->edge);
	free((void*)gtmp->present[0]);
	free((void*)gtmp->present);
	free((void*)gtmp);
	for(t=0; t<graph->sizeTable; t++)
		free((void*)tablePart[t].atom);
	free((void*)tablePart);
	return res;
}
	
/****************************************************************************/
/* General functions on state*/
/****************************************************************************/

void freeCommunityState(TypeCommunityState* state) {
	if(state == NULL)
		return;
	freeMultiGraph(state->graph);
	free((void*)state->edgeList);
	free((void*)state->elementInit);
	free((void*)state->element);
	free((void*)state->community);
	free((void*)state);
}

TypePartition communityStateToPartition(TypeCommunityState *state) {
	TypePartition part;
	int i, ind=0, *index, p;
	index = (int*) malloc(state->sizeElement*sizeof(int));
	for(p=state->first; p>=0; p=state->community[p].next)
		index[p] = ind++;
	part.sizeItem = state->sizeInit;
	part.sizeAtom = state->sizeProto;
	part.atom = (int*) malloc(part.sizeItem*sizeof(int));
	for(i=0; i<state->sizeInit; i++)
		part.atom[i] = index[state->element[state->elementInit[i]].atom];
	free((void*)index);
	return part;
}

TypeMultiGraph *getUpdatedGraph(TypeCommunityState *state, int *index) {
	TypeMultiGraph *g;
	int p, n, m, t;
	g = (TypeMultiGraph*) malloc(sizeof(TypeMultiGraph));
	g->sizeGraph = state->sizeProto;
	g->sizeTable = state->graph->sizeTable;
	g->name = (char**) malloc(g->sizeGraph*sizeof(char*));
	for(n=0; n<g->sizeGraph; n++) {
		char tmp[100];
		sprintf(tmp, "%d", n+1);
		g->name[n] = (char*) malloc((strlen(tmp)+1)*sizeof(char));
		strcpy(g->name[n], tmp);
	}
	g->edge = (TypeEdgeG***) malloc(g->sizeTable*sizeof(TypeEdgeG**));
	g->present = (int**) malloc(g->sizeTable*sizeof(int*));
	for(t=0; t<g->sizeTable; t++) {
		g->present[t] = (int*) malloc(g->sizeGraph*sizeof(int));
		g->edge[t] = (TypeEdgeG**) malloc(g->sizeGraph*sizeof(TypeEdgeG*));
		for(p=state->first; p>=0; p=state->community[p].next) {
			int e, f;
			g->edge[t][index[p]] = (TypeEdgeG*) malloc(g->sizeGraph*sizeof(TypeEdgeG));
			for(m=0; m<g->sizeGraph; m++) {
				g->edge[t][index[p]][m] = 0;
				g->present[t][m] = state->graph->present[t][m];
			}
			for(e=state->community[p].first; e>=0; e=state->element[e].next)
				for(f=0; f<state->graph->sizeGraph; f++)
					g->edge[t][index[p]][index[state->element[f].atom]] += state->graph->edge[t][e][f];
		}
	}
	return g;
}

TypeCommunityState *initCommunityState(TypeMultiGraph *graph) {
	TypeCommunityState *state;
	int i, j, *prec, iedge;
	state = (TypeCommunityState*) malloc(sizeof(TypeCommunityState));
	state->graph = cpyMultiGraph(graph);
	state->gsave = graph;
	state->sizeInit = graph->sizeGraph;
	state->sizeElement = graph->sizeGraph;
	state->sizeProto = state->sizeElement;
	state->element = (TypeElementClass*) malloc(state->sizeElement*sizeof(TypeElementClass));
	state->community = (TypeProtoClass*) malloc(state->sizeElement*sizeof(TypeProtoClass));
	state->sizeInit = state->sizeElement;
	state->elementInit = (int*) malloc(state->sizeInit*sizeof(int));
	state->first = 0;
	state->trash = -1;
	state->nedge = countEdge(graph);
	state->nedgeTot = (graph->sizeGraph*(graph->sizeGraph-1))/2;
	prec = &(state->first);
	state->edgeList = (TypeElementEdge*) malloc(2*state->nedge*sizeof(TypeElementEdge));
	iedge = 0;
	for(i=0; i<state->sizeElement; i++) {
		state->elementInit[i] = i;
		state->community[i].size = 1;
		state->community[i].first = i;
		state->community[i].prec = prec;
		state->community[i].next = i+1;
		prec = &(state->community[i].next); 
		state->element[i].size = 1;
		state->element[i].atom = i;
		state->element[i].prec = &(state->community[i].first);
		state->element[i].next = -1;
		state->element[i].first = -1;
		for(j=0; j<state->sizeElement; j++) {
			int t;
			for(t=0; t<graph->sizeTable && graph->edge[t][i][j] == 0; t++);
			if(t<graph->sizeTable) {
				state->edgeList[iedge].neighbour = j;
				state->edgeList[iedge].next = state->element[i].first;
				state->element[i].first = iedge;
				iedge++;
			}
		}
	}
	state->community[state->sizeElement-1].next = -1;
	return state;
}

TypeCommunityState *getUpdatedCommunityState(TypeCommunityState *state) {
	TypeCommunityState *new;
	int p, *index, ind = 0, i, j, *prec, nedge, iedge;;
	index = (int*) malloc(state->sizeElement*sizeof(int));
	for(p=state->first; p>=0; p=state->community[p].next)
		index[p] = ind++;
/*
printf("CMP %d - %d\n", state->sizeProto, ind);
if(ind != state->sizeProto) {
	int a;
for(a=state->first; a>=0; a=state->community[a].next)
	printf("%d ", a);
printf("F\n");
for(a=state->trash; a>=0; a=state->community[a].next)
	printf("%d ", a);
printf("T\n");
exit(1);
}
*/
	new = (TypeCommunityState*) malloc(sizeof(TypeCommunityState));
	new->graph = getUpdatedGraph(state, index);
	new->gsave = state->gsave;
	new->sizeElement = state->sizeProto;
	new->sizeProto = new->sizeElement;
	new->element = (TypeElementClass*) malloc(new->sizeElement*sizeof(TypeElementClass));
	new->community = (TypeProtoClass*) malloc(new->sizeElement*sizeof(TypeProtoClass));
	new->sizeInit = state->sizeInit;
	new->elementInit = (int*) malloc(new->sizeInit*sizeof(int));
	for(i=0; i<new->sizeInit; i++)
		new->elementInit[i] = index[state->element[state->elementInit[i]].atom];
	new->first = 0;
	new->trash = -1;
	prec = &(new->first);
	nedge = countEdge(new->graph);
	new->edgeList = (TypeElementEdge*) malloc(2*nedge*sizeof(TypeElementEdge));
	iedge = 0;
	for(p=state->first; p>=0; p=state->community[p].next) {
		new->community[index[p]].size = state->community[p].size;
		new->community[index[p]].first = index[p];
		new->community[index[p]].prec = prec;
		if(state->community[p].next >=0)
			new->community[index[p]].next = index[state->community[p].next];
		else
			new->community[index[p]].next = -1;
		prec = &(new->community[index[p]].next); 
		new->element[index[p]].size = state->community[p].size;
		new->element[index[p]].atom = index[p];
		new->element[index[p]].prec = &(new->community[index[p]].first);
		new->element[index[p]].next = -1;
		new->element[index[p]].first = -1;
		for(j=0; j<index[p]; j++) {
			int t;
			for(t=0; t<new->graph->sizeTable && new->graph->edge[t][index[p]][j] == 0; t++);
			if(t < new->graph->sizeTable) {
				new->edgeList[iedge].neighbour = j;
				new->edgeList[iedge].next = new->element[index[p]].first;
				new->element[index[p]].first = iedge;
				iedge++;
			}
		}
		for(j=index[p]+1; j<new->sizeElement; j++) {
			int t;
			for(t=0; t<new->graph->sizeTable && new->graph->edge[t][index[p]][j] == 0; t++);
			if(t < new->graph->sizeTable) {
				new->edgeList[iedge].neighbour = j;
				new->edgeList[iedge].next = new->element[index[p]].first;
				new->element[index[p]].first = iedge;
				iedge++;
			}
		}
	}
	free((void*) index);
//printClassPartition(new);
	return new;
}


void printCommunityState(TypeCommunityState *state) {
	TypePartition part;
	int i;
	part.sizeItem = state->sizeElement;
	part.sizeAtom = state->sizeProto;
	part.atom = (int*) malloc(part.sizeItem*sizeof(int));
	for(i=state->first; i>=0; i = state->community[i].next) {
		int e;
		printf("atom %d : ", i);
		for(e=state->community[i].first; e>=0; e=state->element[e].next)
			printf("%d ", e);
		printf("\n");
	}
}

void printCommunityStateBis(TypeCommunityState *state) {
	int i, n = 0;
	for(i=state->first; i>=0; i = state->community[i].next) {
		printf("%d ", i);
		n++;
	}
	printf("P\n");
	for(i=state->trash; i>=0; i = state->community[i].next) {
		printf("%d ", i);
		n++;
	}
	printf("T\n");
}

int countCommunity(TypeCommunityState *state) {
	int i, n = 0;
	for(i=state->first; i>=0; i=state->community[i].next)
		n++;
	return n;
}

/*test if e is the single element of protoatom i*/
int isSingle(int e, TypeCommunityState *state) {
	return (state->element[e].prec == &(state->community[state->element[e].atom].first) && state->element[e].next < 0);
}

/*transfer element e from protoatom i to protoatom j  !! e must be in protoatom i !!*/
void transfertStandard(int e, int j, TypeCommunityState *state, void *param) {
	int i = state->element[e].atom;
	if(i == j)
		return;
	if(isSingle(e, state)) {
		*(state->community[i].prec) = state->community[i].next;
		if(state->community[i].next >= 0)
			state->community[state->community[i].next].prec = state->community[i].prec;
		state->sizeProto--;
		if(state->trash >=0)
			state->community[state->trash].prec = &(state->community[i].next);
		state->community[i].next = state->trash;
		state->community[i].prec = &(state->trash);
		state->trash = i;
	}
	state->community[i].size -= state->element[e].size;
	state->element[e].atom = j;
	*(state->element[e].prec) = state->element[e].next;
	if(state->element[e].next>=0)
		state->element[state->element[e].next].prec = state->element[e].prec;
	if(state->community[j].first>=0)
		state->element[state->community[j].first].prec = &(state->element[e].next);
	state->element[e].next = state->community[j].first;
	state->element[e].prec = &(state->community[j].first);
	state->community[j].first = e;
	state->community[j].size += state->element[e].size;
}


/****************************************************************************/
/* Generic functions */
/****************************************************************************/

double variationGeneric(int e, int j, TypeCommunityState *state, void *param) {
	int f, t;
	double var = 0.;
	if(state->element[e].atom == j)
		return 0.;
	for(t=0; t<state->graph->sizeTable; t++) {
		for(f=state->community[state->element[e].atom].first; f>=0; f=state->element[f].next)
			if(f != e) {
				var -= ((TypeGenericParamMulti*)param)->table[t].score[e][f];
			}
	}
	for(t=0; t<state->graph->sizeTable; t++) {
		for(f=state->community[j].first; f>=0; f=state->element[f].next) {
			var += ((TypeGenericParamMulti*)param)->table[t].score[e][f];
		}
	}
	return var;
}

TypeCommunityState *getUpdatedCommunityStateParamGeneric(TypeCommunityState *state, void **param) {
	TypeCommunityState *new;
	TypeGenericParamMulti *oldParam, *newParam;
	int p, *index, ind = 0, i, j, *prec, nedge, iedge, t;
	index = (int*) malloc(state->sizeElement*sizeof(int));
	for(p=state->first; p>=0; p=state->community[p].next)
		index[p] = ind++;
	new = (TypeCommunityState*) malloc(sizeof(TypeCommunityState));
	new->graph = getUpdatedGraph(state, index);
	new->gsave = state->gsave;
	new->sizeElement = state->sizeProto;
	new->sizeProto = new->sizeElement;
	new->element = (TypeElementClass*) malloc(new->sizeElement*sizeof(TypeElementClass));
	new->community = (TypeProtoClass*) malloc(new->sizeElement*sizeof(TypeProtoClass));
	new->sizeInit = state->sizeInit;
	new->elementInit = (int*) malloc(new->sizeInit*sizeof(int));
	for(i=0; i<new->sizeInit; i++)
		new->elementInit[i] = index[state->element[state->elementInit[i]].atom];
	new->first = 0;
	new->trash = -1;
	prec = &(new->first);
	nedge = countEdge(new->graph);
	new->edgeList = (TypeElementEdge*) malloc(2*nedge*sizeof(TypeElementEdge));
	iedge = 0;
	for(p=state->first; p>=0; p=state->community[p].next) {
		new->community[index[p]].size = state->community[p].size;
		new->community[index[p]].first = index[p];
		new->community[index[p]].prec = prec;
		if(state->community[p].next >=0)
			new->community[index[p]].next = index[state->community[p].next];
		else
			new->community[index[p]].next = -1;
		prec = &(new->community[index[p]].next); 
		new->element[index[p]].size = state->community[p].size;
		new->element[index[p]].atom = index[p];
		new->element[index[p]].prec = &(new->community[index[p]].first);
		new->element[index[p]].next = -1;
		new->element[index[p]].first = -1;
		for(j=0; j<index[p]; j++) {
			int t;
			for(t=0; t<new->graph->sizeTable && new->graph->edge[t][index[p]][j] == 0; t++);
			if(t < new->graph->sizeTable) {
				new->edgeList[iedge].neighbour = j;
				new->edgeList[iedge].next = new->element[index[p]].first;
				new->element[index[p]].first = iedge;
				iedge++;
			}
		}
		for(j=index[p]+1; j<new->sizeElement; j++) {
			int t;
			for(t=0; t<new->graph->sizeTable && new->graph->edge[t][index[p]][j] == 0; t++);
			if(t < new->graph->sizeTable) {
				new->edgeList[iedge].neighbour = j;
				new->edgeList[iedge].next = new->element[index[p]].first;
				new->element[index[p]].first = iedge;
				iedge++;
			}
		}
	}
	oldParam = (TypeGenericParamMulti*) (*param);
	newParam = (TypeGenericParamMulti*) malloc(sizeof(TypeGenericParamMulti));
	newParam->size = state->graph->sizeTable;
	newParam->sizeBuf = state->graph->sizeTable;
	newParam->sizeGraph = new->graph->sizeGraph;
	newParam->table = (TypeGenericParam*) malloc(newParam->size*sizeof(TypeGenericParam));
	for(t=0; t<state->graph->sizeTable; t++) {
		int e;
		newParam->table[t].score= (double**) malloc(new->graph->sizeGraph*sizeof(double*));
		for(e=0; e<new->graph->sizeGraph; e++) {
			int f;
			newParam->table[t].score[e] = (double*) malloc(new->graph->sizeGraph*sizeof(double));
			for(f=0; f<new->graph->sizeGraph; f++) {
				newParam->table[t].score[e][f] = 0.;
			}
		}
		for(p=state->first; p>=0; p=state->community[p].next) {
			int q;
			for(q=state->first; q>=0; q=state->community[q].next) {
				int e, f;
				newParam->table[t].score[index[p]][index[q]] = 0;
				for(e=state->community[p].first; e>=0; e=state->element[e].next)
					for(f=state->community[q].first; f>=0; f=state->element[f].next) {
						newParam->table[t].score[index[p]][index[q]] += oldParam->table[t].score[e][f];
					}
			}
		}
	}
	freeParamGeneric(*param);
	*param = newParam;
	free((void*) index);
	return new;
}


void freeParamGeneric(void *param) {
	int t;
	if(param != NULL) {
		for(t=0; t<((TypeGenericParamMulti*)param)->sizeBuf; t++) {
			int e;
			if(((TypeGenericParamMulti*)param)->table[t].score != NULL) {
				for(e=0; e<((TypeGenericParamMulti*)param)->sizeGraph; e++) {
					if(((TypeGenericParamMulti*)param)->table[t].score[e] != NULL)
						free((void*)((TypeGenericParamMulti*)param)->table[t].score[e]);
				}
				free((void*)((TypeGenericParamMulti*)param)->table[t].score);
			}
		}
		if(((TypeGenericParamMulti*)param)->table != NULL)
			free((void*)((TypeGenericParamMulti*)param)->table);
		free((void*)param);
	}
}

void updateGeneric(TypeCommunityState **state, void **param) {
	TypeCommunityState *stateTmp = *state;
	*state = getUpdatedCommunityStateParamGeneric(*state, param);
	freeCommunityState(stateTmp);
}



/*NewmanBis functions*/

void *getParamNewman(TypeCommunityState *state, void *info) {
	int t;
	TypeGenericParamMulti *param;
	double *tmp, m;
	tmp = (double*) malloc(state->graph->sizeGraph*sizeof(double));
	param = (TypeGenericParamMulti*) malloc(sizeof(TypeGenericParamMulti));
	param->size = state->graph->sizeTable;
	param->sizeGraph = state->graph->sizeGraph;
	param->sizeBuf = state->graph->sizeTable;
	param->table = (TypeGenericParam*) malloc(param->size*sizeof(TypeGenericParam));
	for(t=0; t<state->graph->sizeTable; t++) {
		int i,j;
		m = 0.;
		for(i=0; i<state->graph->sizeGraph; i++) {
			tmp[i] = 0.;
			for(j=0; j<state->graph->sizeGraph; j++)
				tmp[i] += (double) state->graph->edge[t][i][j];
			m += tmp[i];
		}
		m /= 2.;
		param->table[t].score= (double**) malloc(state->graph->sizeGraph*sizeof(double*));
		for(i=0; i<state->graph->sizeGraph; i++) {
			param->table[t].score[i] = (double*) malloc(state->graph->sizeGraph*sizeof(double));
			param->table[t].score[i][i] = 0.;
			for(j=0; j<i; j++) {
				param->table[t].score[i][j] = (((double)state->graph->edge[t][i][j])-(tmp[i]*tmp[j])/(2.*m))/(2.*m);
				param->table[t].score[j][i] = param->table[t].score[i][j];
			}
		}
	}
	free((void*)tmp);
	return (void*) param;
}

void initNewman(TypeMultiGraph *graph, void *info, TypeCommunityState **stateP, void **paramP) {
	if(graph->sizeGraph > 0) {
		*stateP = initCommunityState(graph);
		*paramP = getParamNewman(*stateP, info);
	} else {
		*stateP = NULL;
		*paramP = NULL;
	}
}


/****************************************************************************/
/* Louvain functions */
/****************************************************************************/
double variationLouvain(int e, int j, TypeCommunityState *state, void *param) {
	int f, t;
	double var = 0.;
	if(state->element[e].atom == j)
		return 0.;
	for(t=0; t<state->graph->sizeTable; t++) {
		double sum = 0.;
		for(f=state->community[state->element[e].atom].first; f>=0; f=state->element[f].next)
			if(f != e)
				sum += (((TypeLouvainParamMulti*)param)->alpha*((TypeLouvainParamMulti*)param)->table[t].k[f]*((TypeLouvainParamMulti*)param)->table[t].k[e])/(2.*((TypeLouvainParamMulti*)param)->table[t].m)-state->graph->edge[t][e][f];
		var += ((TypeLouvainParamMulti*)param)->weight[t]*sum/(2.*((TypeLouvainParamMulti*)param)->table[t].m);
	}
	for(t=0; t<state->graph->sizeTable; t++) {
		double sum = 0.;
		for(f=state->community[j].first; f>=0; f=state->element[f].next)
			sum += state->graph->edge[t][e][f]-(((TypeLouvainParamMulti*)param)->alpha*((TypeLouvainParamMulti*)param)->table[t].k[f]*((TypeLouvainParamMulti*)param)->table[t].k[e])/(2.*((TypeLouvainParamMulti*)param)->table[t].m);
		var += ((TypeLouvainParamMulti*)param)->weight[t]*sum/(2.*((TypeLouvainParamMulti*)param)->table[t].m);
	}
	return var;
}

void computeParamLouvain(TypeCommunityState *state, TypeLouvainParamMulti *param) {
	int t;
	param->size = state->graph->sizeTable;
	for(t=0; t<state->graph->sizeTable; t++) {
		int e, f;
		param->table[t].m = 0.;
		for(e=0; e<state->graph->sizeGraph; e++) {
			param->table[t].k[e] = 0.;
			for(f=0; f<state->graph->sizeGraph; f++)
				param->table[t].k[e] += state->graph->edge[t][e][f];
			param->table[t].m += param->table[t].k[e];
		}
		param->table[t].m /= 2.;
	}
}

void *getParamLouvain(TypeCommunityState *state, void *info) {
	int t;
	TypeLouvainParamMulti *param;
	param = (TypeLouvainParamMulti*) malloc(sizeof(TypeLouvainParamMulti));
	param->size = state->graph->sizeTable;
	param->sizeBuf = state->graph->sizeTable;
	param->table = (TypeLouvainParam*) malloc(param->size*sizeof(TypeLouvainParam));
	for(t=0; t<state->graph->sizeTable; t++)
		param->table[t].k = (double*) malloc(state->graph->sizeGraph*sizeof(double));
	computeParamLouvain(state, param);
	if(info != NULL) {
		param->alpha = ((TypeLouvainInfo*) info)->alpha;
		param->weight = ((TypeLouvainInfo*) info)->weight;
	}
	return (void*) param;
}

void freeParamLouvain(void *param) {
	int t;
	if(param == NULL)
		return;
	for(t=0; t<((TypeLouvainParamMulti*)param)->sizeBuf; t++)
		free((void*)((TypeLouvainParamMulti*)param)->table[t].k);
	free((void*)((TypeLouvainParamMulti*)param)->table);
	free((void*)param);
}

void updateLouvain(TypeCommunityState **state, void **param) {
	TypeCommunityState *stateTmp = *state;
	*state = getUpdatedCommunityState(*state);
	freeCommunityState(stateTmp);
	computeParamLouvain(*state, *param);
}

void initLouvain(TypeMultiGraph *graph, void *info, TypeCommunityState **stateP, void **paramP) {
	*stateP = initCommunityState(graph);
	*paramP = getParamLouvain(*stateP, info);
}

/****************************************************************************/
/* Global functions */
/****************************************************************************/

int iterateGlobal(TypeLouvainGlobal *global) {
	int i, e, change = 0, *done;
	done = (int*) malloc(global->state->sizeElement*sizeof(int));
	for(i=0; i<global->state->sizeElement; i++)
		done[i] = 0;
	for(e=0; e<global->state->sizeElement; e++) {
		int ie, j, jmax;
		i = global->state->element[e].atom;
		jmax = i;
		double max = 0;
		for(ie=global->state->element[e].first; ie >= 0; ie=global->state->edgeList[ie].next) {
			j = global->state->element[global->state->edgeList[ie].neighbour].atom;
			if(j<0 || j>=global->state->sizeElement)
				printf("XXX %d\t%d\n", global->state->edgeList[ie].neighbour, j);
			if(!done[j]) {
				double gain;
				done[j] = 1;
				gain = global->variation(e, j, global->state, global->param);
				if(gain > max) {
					max = gain;
					jmax = j;
				}
			}
		}
		if(!isSingle(e, global->state)) {
			double gain;
			gain = global->variation(e, global->state->trash, global->state, global->param);
			if(gain > max && gain>EPS) {
				int tmp;
				max = gain;
				jmax = global->state->trash;
				tmp = global->state->community[global->state->trash].next;
				global->state->community[global->state->trash].prec = &(global->state->first);
				global->state->community[global->state->trash].next = global->state->first;
				if(global->state->first>=0)
					global->state->community[global->state->first].prec = &(global->state->community[global->state->trash].next);
				global->state->first = global->state->trash;
				if(tmp>=0)
					global->state->community[tmp].prec = &(global->state->trash);
				global->state->trash = tmp;
				global->state->sizeProto++;
			}
		}
		if(max>EPS) {
			global->transfert(e, jmax, global->state, global->param);
			change = 1;
		}
		for(ie=global->state->element[e].first; ie >= 0; ie=global->state->edgeList[ie].next)
			done[global->state->element[global->state->edgeList[ie].neighbour].atom] = 0;
	}
	free((void*)done);
	return change;
}

int iterateGlobalRandomized(TypeLouvainGlobal *global) {
	int i, e, change = 0, *done;
	done = (int*) malloc(global->state->sizeElement*sizeof(int));
	for(i=0; i<global->state->sizeElement; i++)
		done[i] = 0;
	for(e=0; e<global->state->sizeElement; e++) {
		int ie, j, jmax, *jtab, jsize;
		i = global->state->element[e].atom;
		jsize = 0;
		for(ie=global->state->element[e].first; ie >= 0; ie=global->state->edgeList[ie].next)
			jsize++;
		jtab = (int*) malloc((jsize+1)*sizeof(int));
		jsize = 0;
		for(ie=global->state->element[e].first; ie >= 0; ie=global->state->edgeList[ie].next) {
			j = global->state->element[global->state->edgeList[ie].neighbour].atom;
			if(j<0 || j>=global->state->sizeElement)
				printf("XXX %d\t%d\n", global->state->edgeList[ie].neighbour, j);
			if(!done[j]) {
				double gain;
				done[j] = 1;
				gain = global->variation(e, j, global->state, global->param);
				if(gain > EPS) {
					jtab[jsize++] = j;
				}
			}
		}
		if(!isSingle(e, global->state)) {
			double gain = global->variation(e, global->state->trash, global->state, global->param);
			if(gain > EPS) {
				jtab[jsize++] = global->state->trash;
			}
		}
		if(jsize>0) {
			jmax = jtab[randAtMost(jsize-1)];
			if(jmax == global->state->trash) {
				int tmp;
				tmp = global->state->community[global->state->trash].next;
				global->state->community[global->state->trash].prec = &(global->state->first);
				global->state->community[global->state->trash].next = global->state->first;
				if(global->state->first>=0)
					global->state->community[global->state->first].prec = &(global->state->community[global->state->trash].next);
				global->state->first = global->state->trash;
				if(tmp>=0)
					global->state->community[tmp].prec = &(global->state->trash);
				global->state->trash = tmp;
				global->state->sizeProto++;
			}
			global->transfert(e, jmax, global->state, global->param);
			change = 1;
		}
		free((void*)jtab);
		for(ie=global->state->element[e].first; ie >= 0; ie=global->state->edgeList[ie].next)
			done[global->state->element[global->state->edgeList[ie].neighbour].atom] = 0;
	}
	free((void*)done);
	return change;
}

TypePartition getPartition(TypeMultiGraph *graph, TypePartitionMethod type, TypeIterFunc iter, void *info) {
	TypeLouvainGlobal *global;
	TypePartition part;
	int cont;
	global = (TypeLouvainGlobal*) malloc(sizeof(TypeLouvainGlobal));
	switch(type) {
		case NewmanType:
			global->variation = variationGeneric;
			global->init = initNewman;
			global->update = updateGeneric;
			global->transfert = transfertStandard;
			global->freeParam = freeParamGeneric;
			break;
		case LouvainType:
		default:
			global->variation = variationLouvain;
			global->init = initLouvain;
			global->update = updateLouvain;
			global->transfert = transfertStandard;
			global->freeParam = freeParamLouvain;
			break;
	}
	global->init(graph, info, &(global->state), &(global->param));
	do {
		while(iter(global))
		;
		cont = (global->state->sizeProto != global->state->sizeElement);
		if(cont)
			global->update(&global->state, &global->param);
	} while(cont);
	part = communityStateToPartition(global->state);
	freeCommunityState(global->state);
	global->freeParam(global->param);
	free((void*)global);
	return part;
}

#!/usr/bin/env python

import random
from random import randrange
import sys
from math import exp, fabs
from operator import add
from time import time
from datetime import datetime
from heapq import nlargest, nsmallest
#import string
#from string import *
import os
from os import system

##### Global Variables #####

model_pathfile=""
model_file=""
if_predict_all=0
logfile_fd = ""

whole_fsc_dict={}
whole_imp_v=[]


def arg_process():
	global model_pathfile, map_pathfile
	global model_file, map_file
	

	if len(sys.argv) not in [3]:
		print('Usage: %s model_file map_file' % sys.argv[0])
		raise SystemExit

	model_pathfile=sys.argv[1]
	assert os.path.exists(model_pathfile),"model file not found"
	model_file = os.path.split(model_pathfile)[1]

	map_pathfile=sys.argv[2]
	assert os.path.exists(map_pathfile),"map file not found"
	map_file = os.path.split(map_pathfile)[1]





##### Log related #####
def initlog(name):
	global logname
	logname = name
	logfile_fd = open(logname, 'w')
	logfile_fd.close()


VERBOSE_MAX=100
VERBOSE_ITER = 3
VERBOSE_GRID_TIME = 2
VERBOSE_TIME = 1

def writelog(str, vlevel=VERBOSE_MAX):
	global logname
	global logfile_fd
	if vlevel > VERBOSE_ITER:
		if not logfile_fd:
			logfile_fd = open(logname, 'a')
		logfile_fd.write(str)
		#logfile_fd.close()


def rem_file(filename):
	system("rm -f %s"%filename)
	#unlink(filename)

def main():
	global train_pathfile, train_file
	global model_pathfile, model_file
	global map_pathfile, map_file
	print("reading model file....")
	t=time()
	params, svs, max_index_model = readmodeldata(model_pathfile)
	t=time()-t
	print("%s"%(params["kernel_type"]))
	print("total support vectors: %i"%(len(svs)))
	print("read done")
	print("reading feature map file....")
	t=time()	
	fmap = readmapdata(map_pathfile)
	t=time()-t
	if params["kernel_type"] != "linear":
		print("Kernel %s not supported."%(params["kernel_type"]))
		exit(1)
	w = [0 for i in range(max_index_model)]
	for i in range(max_index_model):
		for sv in svs:
			if (i+1) in sv.svec:
				w[i] += sv.ag*sv.svec[i+1] 
	w_i = [(i+1,w[i]) for i in range(max_index_model)]
	w_i.sort(key = lambda(i,w) : abs(w), reverse=True)
	for (i,w) in w_i:
		print("w(%i:%s) = %f"%(i, fmap[i], w))

		
	

def printMappedVector(vector, map):
	index = vector.svec.keys()
	index.sort(key = lambda i: map[i])
	for (i) in index:
		print("\t(%i) %s "%(i, map[i]))
	
def printMappedSampleAndVector(vector, sample, map):
	indexv = vector.svec.keys()
	indexs = sample.svec.keys()
	indexv.sort()
	indexs.sort()
	left = "Support Vector({1}) |u|^2={0}".format(vector.norm_sq, vector.id).ljust(35)
	right = "Sample Vector |v|^2={0}".format(sample.norm_sq).ljust(35)
	print("\t%s\t%s"%(left,right))
	left = "ag={0} f_matches={1}".format(vector.ag, vector.prod(sample)).ljust(35)
	right = "".format(sample.norm_sq).ljust(35)
	print("\t%s\t%s\n"%(left,right))
	v = 0
	s = 0
	while v < len(indexv) and s < len(indexs):
		if indexv[v] == indexs[s]:
			left = "({0}) {1}".format(indexv[v], map[indexv[v]]).ljust(35)
			right = "({0}) {1}".format(indexs[s], map[indexs[s]]).ljust(35)
			print("\t%s\t%s"%(left,right))
			v += 1
			s += 1
		elif indexv[v] < indexs[s]:
			left = "({0}) {1}".format(indexv[v], map[indexv[v]]).ljust(35)
			right = ""
			print("\t%s\t%s"%(left,right))
			v += 1
		elif indexs[s] < indexv[v]:
			left = "".ljust(35)
			right = "({0}) {1}".format(indexs[s], map[indexs[s]]).ljust(35)
			print("\t%s\t%s"%(left,right))
			s += 1
	if s < len(indexs):
		while s < len(indexs):
			left = "".ljust(35)
			right = "({0}) {1}".format(indexs[s], map[indexs[s]]).ljust(35)
			print("\t%s\t%s"%(left,right))
			s += 1
	if v < len(indexv):
		while v < len(indexv):
			left = "({0}) {1}".format(indexv[v], map[indexv[v]]).ljust(35)
			right = ""
			print("\t%s\t%s"%(left,right))
			v += 1			


###### svm data IO ######
def readmapdata(filename):
	map={}
	#load training data
	fp = open(filename)
	line = fp.readline()
	while line:
		line=line.strip()
		id, value = line.split()
		map[int(id)] = value
		line = fp.readline()
		continue
	return map

def readsampledata(filename):
	samples=[]
	max_index=0
	#load training data
	fp = open(filename)
	line = fp.readline()
	while line:
		# added by untitled, allowing data with comments
		line=line.strip()
		if line[0]=="#":
			line = fp.readline()
			continue
		elems = line.split()
		sv = TVector()
		for e in elems[1:]:
			points = e.split(":")
			p0 = int( points[0].strip() )
			p1 = float( points[1].strip() )
			sv.svec[p0] = p1
			if p0 > max_index:
				max_index = p0
		sv.label = float(elems[0])
		sv.norm_sq = sv.prod(sv)
		samples.append(sv)
		line = fp.readline()
	fp.close()
	return samples, max_index

class rbf_kernel():
	def __init__(self, gamma):
		self.gamma = gamma
	def k(self, sv, sample):
		result = exp(-1*self.gamma*(sv.norm_sq+sample.norm_sq-2*sv.prod(sample)))
		return result
	
class linear_kernel():
	def __init__(self):
		next
		
	def k(self, sv, sample):
		result = sv.prod(sample)
		return result
	
class Vector():
	def __init__(self):
		self.svec = {}
		self.norm_sq = 0
	def prod(self, sv):
		indexa = self.svec.keys()
		indexb = sv.svec.keys()
		indexa.sort()
		indexb.sort()
		a = 0
		b = 0
		sum = float(0.0)
		while a < len(indexa) and b < len(indexb):
			if indexa[a] < indexb[b]:
				a = a + 1
			elif indexb[b] < indexa[a]:
				b = b + 1
			else:
				sum += self.svec[indexa[a]] * sv.svec[indexb[b]]
				a = a + 1
				b = b + 1
		return float(sum)

class TVector(Vector):
	def __init__(self):
		self.label = 0.0
		self.svec = {}

class SVector(Vector):
	def __hash__(self):
		return hash(self.id)
	def __cmp__(self,x):
		if self.id < x.id:
			return -1
		if x.id < self.id:
			return 1
		return 0
	def __init__(self):
		self.ag = 0.0
		self.id = -1
		self.svec = {}
	def cal_pull(self, sample, kernel):
		k = kernel(self, sample)
		return self.ag * k

def readmodeldata(filename):
	params={}
	svs=[]
	max_index=0
	#load training data
	fp = open(filename)
	line = fp.readline()
	svstart = False
	id = 1
	while line:
		# added by untitled, allowing data with comments
		line=line.strip()
		if line[0]=="#":
			line = fp.readline()
			continue
		
		if	svstart == False: 
			param = line.split()
			if param[0] == "SV":
				svstart = True
			else:
				params[param[0]] = param[1].strip()
			line = fp.readline()
			continue	
				
		elems = line.split()
		sv = SVector()
		for e in elems[1:]:
			points = e.split(":")
			p0 = int( points[0].strip() )
			p1 = float( points[1].strip() )
			sv.svec[p0] = p1
			if p0 > max_index:
				max_index = p0
		sv.ag = float(elems[0])
		sv.id = id
		id += 1
		sv.norm_sq = sv.prod(sv)
		svs.append(sv)
		line = fp.readline()
	fp.close()
	return params,svs,max_index


###### PROGRAM ENTRY POINT ######

arg_process()
main()



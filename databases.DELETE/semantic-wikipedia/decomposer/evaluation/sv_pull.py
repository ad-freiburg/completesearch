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

train_pathfile=""
train_file=""
model_pathfile=""
model_file=""
if_predict_all=0
logfile_fd = ""

whole_fsc_dict={}
whole_imp_v=[]


def arg_process():
	global model_pathfile, train_pathfile, map_pathfile
	global model_file, train_file, map_file
	

	if len(sys.argv) not in [4]:
		print('Usage: %s model_file training_file map_file' % sys.argv[0])
		raise SystemExit

	model_pathfile=sys.argv[1]
	assert os.path.exists(model_pathfile),"model file not found"
	model_file = os.path.split(model_pathfile)[1]

	train_pathfile=sys.argv[2]
	assert os.path.exists(train_pathfile),"training file not found"
	train_file = os.path.split(train_pathfile)[1]
	
	map_pathfile=sys.argv[3]
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
	writelog("loading data '%s': %.1f sec.\n"%(model_pathfile,t))
	print("%s"%(params["kernel_type"]))
	print("total support vectors: %i"%(len(svs)))
	writelog("total support vectors: %i\n"%(len(svs)))
	print("read done")
	print("reading training file....")
	t=time()
	samples, max_index_train = readsampledata(train_pathfile)
	t=time()-t
	writelog("loading data '%s': %.1f sec.\n"%(train_pathfile,t))
	print("total samples: %i"%(len(samples)))
	writelog("total samples: %i\n"%(len(samples)))
	print("read done")
	print("reading feature map file....")
	t=time()	
	fmap = readmapdata(map_pathfile)
	t=time()-t
	writelog("loading data '%s': %.1f sec.\n"%(map_pathfile,t))
	if params["kernel_type"] == "rbf":
		rbf_gamma = float(params["gamma"])
		print ("RBF kernel with gamma: %s"%(rbf_gamma))
		writelog ("RBF kernel with gamma: %s\n"%(rbf_gamma))
		kernel = rbf_kernel(rbf_gamma)
	elif params["kernel_type"] == "linear":
		kernel = linear_kernel()
		print ("Linear kernel")
		writelog ("Linear kernel\n")
	else:
		print("Kernel %s not supported."%(params["kernel_type"]))
		exit(1)
	i = 1
	n = 30
	all_tops_pos = dict.fromkeys(svs, 0)
	all_tops_neg = dict.fromkeys(svs, 0)
	correct = 0
	false = 0
	results = []
	writelog("Evaluating...\n")
	for sample in samples:
		sum, pulls = classify_get_pulls(sample, svs, kernel, float(params["rho"]))
		#neg_results = []
		pulls.sort(key = lambda(sv, pull): abs(pull), reverse=True)
		
		#if (sum < 0):
		#	# pulls.sort()
		#	result = nsmallest(n, pulls, key=lambda (sv, pull): pull)
		#else:
		# top = nlargest(n, pulls , key=lambda (sv, pull): pull)
		# pulls = filter(lambda (sv, pull): pull < 0, pulls)
		#neg_pulls = filter(lambda (sv, pull): pull < 0, pulls)
		# for (sv, pull) in top:
		if sum < 0:
			# all_tops_neg[sv] -= 1
			print("Result for sample %i: -1 "%(i))
			results.append((-1,pulls))
			# neg_results.append((-1,neg_pulls))
		else:
			print("Result for sample %i: +1 "%(i))
			results.append((1,pulls))
			#neg_results.append((1,neg_pulls))
			# all_tops_pos[sv] += 1
		writelog("Sample %i\n"%(i))
		for (sv, pull) in pulls:
			#writelog("Support Vector %i : %f \t(matching features: %i, sv dim: %i, sample dim: %i)\n\n"%(sv.id, pull, sv.prod(sample), sv.norm_sq, sample.norm_sq))
			writelog("\tPull: %f\n"%(pull))
			logMappedSampleAndVector(sv, sample, fmap)
			writelog("\n")
			#printMappedVector(sv, fmap)
		#if sum > 0:
		#	for (sv, pull) in top:
		#		print("Pull: %f"%(pull))
		##		printMappedSampleAndVector(sv, sample, fmap)	
		#	raw_input()
		if (sum < 0 and sample.label < 0) or (sum > 0 and sample.label > 0):
			correct += 1
		else:
			false += 1
		# print ("result %i(%s):\t%s"%(i,sample.label, sum))
		i += 1

	print("Computing %i top vectors."%(n))
	F = cal_Fscore_sv(results)
	top_list = []
	map(lambda (sv, fscore): top_list.append((sv,fscore)), F.items())
	top_list.sort(key = lambda(sv, fscore): fscore)
	for (sv, fscore) in top_list:
		print("Support Vector %i,ag = %f, f-score = %f\t"%(sv.id, sv.ag, fscore))
		printMappedVector(sv, fmap)
		print("")
	# all_tops_pos_sorted = []
	# Create list from hashmap
	# map(lambda (sv, pull): all_tops_pos_sorted.append((sv,pull)), all_tops_pos.items())
	# Get nlargest from the list
	# all_tops_pos_sorted = nlargest(n, all_tops_pos_sorted, key=lambda (sv, pull): pull)
	# all_tops_neg_sorted = []
	# map(lambda (sv, pull): all_tops_neg_sorted.append((sv,pull)), all_tops_neg.items())
	# all_tops_neg_sorted = nsmallest(n, all_tops_neg_sorted, key=lambda (sv, pull): pull)
	# print("Done.")
	# for (sv, pull) in all_tops_pos_sorted:
	#	print "Best vectors for class."
		# print sv.id
		# printMappedVector(sv, fmap)
	# for (sv, pull) in all_tops_neg_sorted:
    #   print "Best vectors against class."
	#	print sv.id
	#	printMappedVector(sv, fmap)
	print ("Total classifications correct: %s, incorrect: %s"%(correct, false))
		

def classify_get_pulls(sample, svs, kernel, rho):
	sum = 0
	pulls = []
	for sv in svs:
		pull = sv.cal_pull(sample, kernel.k)
		pulls.append((sv, pull))
		sum += pull
	sum = sum - rho
	sum *= -1
	# map(lambda (pull, sv): top_pulls.update({sv:pull}), pulls[:n])
	return sum, pulls

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

def logMappedSampleAndVector(vector, sample, map):
	indexv = vector.svec.keys()
	indexs = sample.svec.keys()
	indexv.sort()
	indexs.sort()
	left = "Support Vector({1}) |u|^2={0}".format(vector.norm_sq, vector.id).ljust(35)
	right = "Sample Vector |v|^2={0}".format(sample.norm_sq).ljust(35)
	writelog("\t%s\t%s\n"%(left,right))
	left = "ag={0} f_matches={1}".format(vector.ag, vector.prod(sample)).ljust(35)
	right = "".format(sample.norm_sq).ljust(35)
	writelog("\t%s\t%s\n\n"%(left,right))
	v = 0
	s = 0
	while v < len(indexv) and s < len(indexs):
		if indexv[v] == indexs[s]:
			left = "({0}) {1}".format(indexv[v], map[indexv[v]]).ljust(35)
			right = "({0}) {1}".format(indexs[s], map[indexs[s]]).ljust(35)
			writelog("\t%s\t%s\n"%(left,right))
			v += 1
			s += 1
		elif indexv[v] < indexs[s]:
			left = "({0}) {1}".format(indexv[v], map[indexv[v]]).ljust(35)
			right = ""
			writelog("\t%s\t%s\n"%(left,right))
			v += 1
		elif indexs[s] < indexv[v]:
			left = "".ljust(35)
			right = "({0}) {1}".format(indexs[s], map[indexs[s]]).ljust(35)
			writelog("\t%s\t%s\n"%(left,right))
			s += 1
	if s < len(indexs):
		while s < len(indexs):
			left = "".ljust(35)
			right = "({0}) {1}".format(indexs[s], map[indexs[s]]).ljust(35)
			writelog("\t%s\t%s\n"%(left,right))
			s += 1
	if v < len(indexv):
		while v < len(indexv):
			left = "({0}) {1}".format(indexv[v], map[indexv[v]]).ljust(35)
			right = ""
			writelog("\t%s\t%s\n"%(left,right))
			v += 1		

###return a dict containing F_j
def cal_Fscore_sv(results):

	# labels, sv_pulllist = results
	
	data_num=float(len(results)) 
	p_num = {} #key: label;  value: data num
	svs = {}
	sum_sv = [] #index: sv_idx;  value: pull sum
	sum_l_sv = {} #dict of lists.  key1: label; index2: sv_idx; value: pull sum
	sumq_l_sv = {} #dict of lists.  key1: label; index2: sv_idx; value: pull sum of square
	F={} #key: feat_idx;  value: fscore
	max_sv_id = 0
	### pass 1: check number of each class and max index of features
	for p in range(len(results)): # for every result
		label, sv_pulllist = results[p]
		if label in p_num: p_num[label] += 1
		else: p_num[label] = 1

		for (sv, pull) in sv_pulllist:
			svs[sv.id] = sv
			if sv.id> max_sv_id:
				max_sv_id = sv.id
	print("MaxSvId=%i"%(max_sv_id))
	### now p_num is set
    
	### initialize variables
	sum_sv = [0 for i in range(max_sv_id)]
	for la in p_num.keys():
		sum_l_sv[la] = [0 for i in range(max_sv_id)]
		sumq_l_sv[la] = [0 for i in range(max_sv_id)]

	### pass 2: calculate some stats of data
	for p in range(len(results)): # for every result
		label, sv_pulllist = results[p]
		for (sv, pull) in sv_pulllist: # for every sv
			sum_sv[sv.id-1] += pull
			sum_l_sv[label][sv.id-1] += pull
			sumq_l_sv[label][sv.id-1] += pull**2
	### now sum_l_sv, sum_l_sv, sumq_l_sv are done

	### for each sv, calculate f-score
	eps = 1e-12
	for id in range(max_sv_id):
		SB = 0
		for la in p_num.keys():
			SB += (p_num[la] * (sum_l_sv[la][id]/p_num[la] - sum_sv[id]/data_num)**2 )

		SW = eps
		for la in p_num.keys():
			SW += (sumq_l_sv[la][id] - (sum_l_sv[la][id]**2)/p_num[la]) 

		if id+1 in svs:
			F[svs[id+1]] = SB / SW

	return F


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

rem_file("%s.log"%model_file)
initlog("%s.log"%model_file)
writelog("start: %s\n\n"%datetime.now())
main()
writelog("\nend: \n%s\n"%datetime.now())


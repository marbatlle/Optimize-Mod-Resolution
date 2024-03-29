import pandas as pd
import numpy as np
import sys

mod_param = pd.read_csv('output/output_mod_parameter.txt',header=None, names=["mod_param"])
num_com = pd.read_csv('output/output_num_communities.txt',header=None, names=["num_communities"])
avg_size = pd.read_csv('output/output_avg_com_size.txt',header=None, names=["avg_community_size"])

data = mod_param.merge(num_com, left_index=True, right_index=True)
data = data.merge(avg_size, left_index=True, right_index=True)
data['num_communities'] = data['num_communities'] - 1 
data["avg_community_size"] = data["avg_community_size"].round()
export_data = data
export_data = export_data.iloc[1: , :]
export_data.to_csv('output/molti-output-analysis.txt', index=None, sep=' ', mode='a') 


original_data = data
data = data[data.avg_community_size != 0]
if data.empty:
    print("     error: Could not define an optimal value for this community structure")
    print(" ")
    print("     error: Could not define an optimal value for this community structure", file=sys.stderr)
    exit()
    

data_subset = data[['num_communities','avg_community_size']]
data_diff = data_subset.diff()
data_diff['result'] = data_diff['avg_community_size']/data_diff['num_communities']
data_result = data_diff[['result']]
data_mod_param = data[["mod_param"]]
data_plot = data_mod_param.merge(data_result, left_index=True, right_index=True)
data_plot = data_plot.iloc[2: , :]

data_plot['result'] = data_plot['result'].fillna(0)

data_plot = data_plot.dropna()
data_plot['result'] = data_plot['result'].abs()

data = data_plot.result.values.tolist()
threshold = 1
for i in range(len(data_plot)):
    if data_plot.result.iloc[i] < threshold:
        optimal_mod_param = data_plot.mod_param.iloc[i]
        break
    else:
        continue
try:
    optimal_mod_param
except NameError:
    min_res = data_plot['result'].min()
    optimal_mod_param = data_plot.loc[data_plot['result'] == min_res, 'mod_param'].iloc[0]


optimal_communities = original_data.loc[original_data['mod_param'] == optimal_mod_param, 'num_communities'].values[0]
optimal_comm_size = original_data.loc[original_data['mod_param'] == optimal_mod_param, 'avg_community_size'].values[0]
print('')
print('Results:')
print('     Optimal modularity parameter:',optimal_mod_param)
print('     - Number of communities:',optimal_communities)
print('     - Average community size:', optimal_comm_size)
print(' ')
import pandas as pd
from datetime import datetime
from collections import defaultdict

# 加载映射文件
try:
    with open('user_id_map.txt', 'r') as f:
        user_id_map = {line.strip().split(',')[0]: int(line.strip().split(',')[1]) for line in f}
    with open('doi_map_file.txt', 'r') as f:
        doi_id_map = {line.strip().split(',')[0]: int(line.strip().split(',')[1]) for line in f}
except FileNotFoundError:
    user_id_map = {}
    doi_id_map = {}

# 创建一个字典来存储每个用户的DOI列表
user_doi_list = defaultdict(list)

# 读取CSV文件
df = pd.read_csv('爬虫结果.csv')

# 更新映射文件和用户-DOI列表
for idx, row in df.iterrows():
    user = row['b']
    doi = row['doi']

    # 如果用户或DOI为空，跳过这一行
    if pd.isna(user) or pd.isna(doi):
        continue

    if user not in user_id_map:
        user_id_map[user] = len(user_id_map) + 1
        with open('user_id_map.txt', 'a') as map_f:
            map_f.write(f"{user},{user_id_map[user]}\n")
    if doi not in doi_id_map:
        doi_id_map[doi] = len(doi_id_map) + 1
        with open('doi_map_file.txt', 'a') as map_f:
            map_f.write(f"{doi},{doi_id_map[doi]}\n")
    user_doi_list[user_id_map[user]].append(doi_id_map[doi])

# 获取今日日期，并转换为字符串格式
today = datetime.today().strftime('%Y-%m-%d')
today='2023-06-01'
# 将结果保存成新的格式
with open(f'{today}_output.txt', 'a') as f:
    for user_id, doi_ids in user_doi_list.items():
        num_dois = len(doi_ids)
        dois = ' '.join(map(str, doi_ids))
        f.write(f"{user_id} {num_dois} {dois}\n")

from transformers import BertModel, BertTokenizer
from sklearn.metrics.pairwise import cosine_similarity
import pandas as pd
import torch
import torch.nn as nn
import csv
from collections import defaultdict
import ast

#定义分类任务类
class classifier(nn.Module):
    def __init__(self):
        #调用父类的构造函数
        super(classifier, self).__init__()
        self.bert = BertModel.from_pretrained(r"C:\Users\99510\Desktop\bert pre")
    def forward(self, input_ids, attention_mask):
        outputs = self.bert(input_ids, attention_mask=attention_mask)
        # 使用CLS token的输出作为句子表示
        cls_output = outputs.last_hidden_state[:, 0, :]
        return cls_output



#make_sentence_bert:将输入的句子(sentense)映射为其对应的id
def make_sentence_bert(sen,tokenizer):
    #返回sentense的编码信息，其中第0维为'input_ids‘,第一维'token_type_ids',第二维'attention_mask'
    tokens = tokenizer.encode_plus(text=sen,max_length=512,padding=True,truncation=True,return_tensors='pt')
    #将句子拆分为token，并不映射为对应的id,out_word为拆分的结果，方便调试用
    out_word=tokenizer.tokenize(sen)#分词，看情况用

    return tokens,out_word
def recommend(user_id, user_preferences, paper_vectors, top_k=5):
    # 获取用户的偏好向量
    user_vector = user_preferences[user_id]
    # 计算与所有论文的余弦相似度
    similarities = {}
    for paper_id, paper_vector in paper_vectors.items():
        similarity = cosine_similarity(user_vector.reshape(1, -1), paper_vector.reshape(1, -1))
        similarities[paper_id] = similarity[0][0]
    # 获取最相似的top_k篇论文
    recommended_papers = sorted(similarities.items(), key=lambda x: x[1], reverse=True)[:top_k]
    return [paper_id for paper_id, similarity in recommended_papers]

model=classifier()
tokenizer = BertTokenizer.from_pretrained(r"C:\Users\99510\Desktop\bert pre")

#单独模块，在未初始化论文doi向量时使用
# #读取论文摘要
# abstract = pd.read_csv('爬虫结果.csv')
# paper_vectors = {}
# for i, row in abstract.iterrows():
#     abstract_text = row['abstract']
#     paper_id = row['doi']
#     if pd.isna(abstract_text):  # 检查摘要是否是nan
#         print(f"Skipping abstract for paper {paper_id} as it is missing.")
#         continue  # 如果是nan，跳过此摘要
#     tokens, _ = make_sentence_bert(abstract_text,tokenizer)
#     with torch.no_grad():
#         vector = model(tokens)
#         paper_vectors[paper_id] = vector
# #保存论文向量为CSV文件
# with open('abstract_vectors.csv', 'w') as f:
#     writer = csv.writer(f)
#     for paper_id, vector in paper_vectors.items():
#         writer.writerow([paper_id] + vector.tolist())
# abstract_dict = abstract.set_index('doi')['abstract'].to_dict()

id_to_doi_map = {}
with open('doi_map_file.txt', 'r') as f:
    reader = csv.reader(f)
    for row in reader:
        doi, paper_id = row
        id_to_doi_map[paper_id] = doi

# 读取论文向量
abstract_vector = pd.read_csv('abstract_vectors.csv',names=['doi', 'vector'])
# 将论文id和向量映射为一个字典，方便查找
paper_vectors = abstract_vector.set_index('doi').to_dict('index')
# 读取用户的浏览记录
with open('2023-06-01_output_2.txt', 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        user_id = int(row[0])
        paper_count = int(row[1])
        paper_ids = [x for x in row[2:2 + paper_count]]
        user_preferences = []
        # 对每篇论文的摘要进行编码，并添加到用户的编码列表中
        for paper_id in paper_ids:
            paper_doi = id_to_doi_map.get(paper_id)  # 从DOI获取paper ID
            if paper_doi is not None:
                vector_dict = paper_vectors.get(paper_doi)
                if vector_dict is not None:
                    vector_list_str = vector_dict['vector']
                    vector_list = ast.literal_eval(vector_list_str)
                    vector = torch.tensor(vector_list, dtype=torch.float)
                    user_preferences[user_id].append(vector)
                    print(vector.shape)
        with open('user_vectors.csv', 'a', newline='') as f2:
            writer = csv.writer(f2)
            # 遍历每个用户及其偏好向量
            if user_preferences:  # 检查向量列表是否为空
                    # 计算平均向量
                average_vector = torch.stack(user_preferences,dim=0)
                average_vector=torch.mean(average_vector)
            else:
                    # 选择其他默认值（例如全0向量或者None）
                average_vector = [0] * 512  # 假设vector_size是您向量的维度
                # 立即写入文件
                writer.writerow([user_id] + average_vector)
                print(f"Processed and saved user {user_id}")
                user_preferences.clear()




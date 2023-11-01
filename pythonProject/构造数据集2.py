import random

# 读取doi_map_file.txt，构建一个doi-id的映射字典
doi_id_map = {}
with open('doi_map_file.txt', 'r') as file:
    for line in file:
        doi, id_ = line.strip().split(',')
        doi_id_map[doi] = id_

# 读取用户浏览数据的txt文件
with open('2023-06-01_output.txt', 'r') as file:
    user_data = [line.strip().split(' ') for line in file]

# 构建新的用户浏览数据
new_user_data = []
for line in user_data:
    user_id = line[0]
    n = int(line[1])
    viewed_articles = line[2:]

    # 随机选择n篇文章，如果重复就重新选择
    new_viewed_articles = viewed_articles[:]
    while len(new_viewed_articles) < 2 * n:
        random_doi = random.choice(list(doi_id_map.keys()))
        random_id = doi_id_map[random_doi]
        if random_id not in new_viewed_articles:
            new_viewed_articles.append(random_id)

    # 把新的浏览记录添加到新的用户数据中
    new_user_data.append([user_id, 2 * n] + new_viewed_articles)

# 把新的用户数据写入到新的txt文件中
with open('2023-06-01_output_2.txt', 'w') as file:
    for line in new_user_data:
        file.write(' '.join(map(str, line)) + '\n')















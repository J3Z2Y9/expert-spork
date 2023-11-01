# #以下为测试代码段，测试爬虫能否爬取文件并解析
# # from selenium import webdriver
# #
# # # 创建一个新的 Chrome WebDriver 实例
# # driver = webdriver.Chrome()
# #
# # # 访问目标网页
# # driver.get('https://link.springer.com/article/10.1007/s11433-022-1989-9')
# #
# # # 获取网页内容
# # html = driver.page_source
# #
# # # 保存网页内容为 HTML 文件
# # with open('output3.html', 'w', encoding='utf-8') as file:
# #     file.write(html)
# #
# # # 关闭 WebDriver 实例
# # driver.quit()

# from bs4 import BeautifulSoup
# import re
#
# # 读取 HTML 文件
# with open('output.html', 'r', encoding='utf-8') as file:
#     html = file.read()
#
# # 创建 BeautifulSoup 对象
# soup = BeautifulSoup(html, 'html.parser')
#
# # 将整个 HTML 文档转换为文本
# text = soup.get_text()
#
# # 使用正则表达式搜索 "DOI:" 后的内容
# match = re.search(r'DOI:\s*(\S+)', text)
#
# if match:
#     # 如果找到匹配项，打印出来
#     print('DOI: ', match.group(1))
# else:
#     print('No DOI found')


#以下为处理单一网址的代码
# from bs4 import BeautifulSoup
# from selenium import webdriver
# import re
#
# # 设置 WebDriver 的路径（Chrome 的示例）
# driver = webdriver.Chrome()
#
# # 访问你想要爬取的 URL
# driver.get('https://link.springer.com/article/10.1007/s12274-016-1317-1')
#
# # 获取页面 HTML
# html = driver.page_source
#
# # 创建 BeautifulSoup 对象
# soup = BeautifulSoup(html, 'html.parser')

# # 将整个 HTML 文档转换为文本
# text = soup.get_text()
#
# # 使用正则表达式搜索 "DOI:" 后的内容
# #match = re.search(r'DOI:\s*(\S+)', text)
# #match = re.search(r'DOI:\s*(\S+?)(?=Publisher:)', text)
# #match = re.search(r'DOI:\s*(\S+)(?:Publisher:)', text)
#
# # 使用正则表达式搜索 "DOI:" 后的内容
# match_doi = re.search(r'DOI:\s*(\S+?)(?=Publisher:)', text)
# # 这里我们使用了零宽断言来确保我们匹配到第二个 "Abstract:" 后的内容，并且不以 "Published in:" 结尾
# match_abstract = re.search(r'(?:.*Abstract:){2}\s*(.*?)(?=Published in:)', text, re.DOTALL)
# # 使用正则表达式搜索 "Published in:" 后的内容
# match_published_in = re.search(r'Published in:\s*(.*?)(?=Page\(s\):)', text)
# # 使用正则表达式搜索 "Date of Publication:" 后的内容
# match_date_of_publication = re.search(r'Date of Publication:\s*(.*?)(?=(ISSN Information:|Electronic ISSN:))', text)
#
# # 输出结果
# if match_doi:
#     print('DOI: ', match_doi.group(1))
# else:
#     print('No DOI found')
# if match_abstract:
#     print('Abstract: ', match_abstract.group(1).strip())
# else:
#     print('No Abstract found')
# if match_published_in:
#     print('Published in: ', match_published_in.group(1))
# else:
#     print('No Published in found')
# if match_date_of_publication:
#     print('Date of Publication: ', match_date_of_publication.group(1))
# else:
#     print('No Date of Publication found')
# abstract_section = soup.find('div', id='Abs1-content')
#
# if abstract_section is not None:
#     # Get the text within the <p> tag in the abstract section
#     abstract_text = abstract_section.p.get_text(strip=True)
# else:
#     abstract_text = "No abstract found"
#
# print(abstract_text)
# # 关闭 WebDriver
# driver.close()

#csv多网址同时
import pandas as pd
from bs4 import BeautifulSoup
from selenium import webdriver
import re

# 读取 CSV 文件
df = pd.read_csv('爬虫结果.csv')  # 将 'filename.csv' 替换为你的 CSV 文件名

# 设置 WebDriver 的路径（Chrome 的示例）
driver = webdriver.Chrome()

for index, row in df.iterrows():
    url = row['c']
    #检查 URL 是否以 'https://ieeexplore.ieee.org' 开头
    # if url.startswith('https://ieeexplore.ieee.org'):
    #     # 访问你想要爬取的 URL
    #     driver.get(url)
    #
    #     # 获取页面 HTML
    #     html = driver.page_source
    #
    #     # 创建 BeautifulSoup 对象
    #     soup = BeautifulSoup(html, 'html.parser')
    #
    #     # 将整个 HTML 文档转换为文本
    #     text = soup.get_text()
    #
    #     # 使用正则表达式搜索 "DOI:" 后的内容
    #     match_doi = re.search(r'DOI:\s*(\S+?)(?=Publisher:)', text)
    #     # 这里我们使用了零宽断言来确保我们匹配到第二个 "Abstract:" 后的内容，并且不以 "Published in:" 结尾
    #     match_abstract = re.search(r'(?:.*Abstract:){2}\s*(.*?)(?=Published in:)', text, re.DOTALL)
    #     # 使用正则表达式搜索 "Published in:" 后的内容
    #     match_published_in = re.search(r'Published in:\s*(.*?)(?=Page\(s\):)', text)
    #     # 使用正则表达式搜索 "Date of Publication:" 后的内容
    #     match_date_of_publication = re.search(r'Date of Publication:\s*(.*?)(?=(ISSN Information:|Electronic ISSN:))', text)
    #
    #     # 将结果保存到 CSV 文件中对应列
    #     if match_doi:
    #         df.loc[index, 'doi'] = match_doi.group(1)
    #     if match_abstract:
    #         df.loc[index, 'abstract'] = match_abstract.group(1).strip()
    #     if match_published_in:
    #         df.loc[index, 'published'] = match_published_in.group(1)
    #     if match_date_of_publication:
    #         df.loc[index, 'date ofpublication'] = match_date_of_publication.group(1)
    if url.startswith('https://link.springer.com'):
        driver.get(url)
        # 获取页面 HTML
        html = driver.page_source
        # 创建 BeautifulSoup 对象
        soup = BeautifulSoup(html, 'html.parser')
        abstract_section = soup.find('div', id='Abs1-content')

        if abstract_section is not None:
            # Get the text within the <p> tag in the abstract section
            abstract_text = abstract_section.p.get_text(strip=True)
            df.loc[index, 'abstract'] = abstract_text
# 关闭 WebDriver
driver.close()

# 保存处理过的 CSV 文件
df.to_csv('爬虫结果.csv', index=False)  # 将 'filename.csv' 替换为你的 CSV 文件名


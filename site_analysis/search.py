from bs4 import BeautifulSoup
from urllib import request, parse
import re
import time


def construct_graph(start_page, name_of_file):
    current_page = start_page
    watched_pages = set()
    new_pages = set()
    new_pages.add(start_page)
    bad_patterns = ['[lL]urkmore', 'index', '^/Главная_страница', '^/Служебная', '^/Категория',
                    '^/Портал', '^/Имена', '^/Обсуждение', '^http', '#', 'wikipedia', 'jpg',
                    'creative', 'Копипаста', 'png', 'Файл', 'MediaWiki', 'www.',
                    '^irc://', '^ftp://', '^news://', 'wiktionary',
                    'mailto', 'telnet', '^/Смехуечки', '^/Шаблон', 'wikimedia', 'Участник']
    counter = 1
    with open(name_of_file, 'a') as out_file:
        start_time = time.time()
        while len(new_pages) > 0:
            current_page = new_pages.pop()
            if current_page in watched_pages:
                continue
            try:
                html_page = request.urlopen(current_page)
            except:
                print(current_page)
                continue
            soup = BeautifulSoup(html_page, 'lxml')
            outgoing_pages = set()
            for link in soup.findAll('a'):
                url = link.get('href')
                if url is not None:
                    unquoted_url = parse.unquote(url)
                    pattern_flag = True
                    table_flag = True
                    for pattern in bad_patterns:
                        if len(re.findall(pattern, unquoted_url)) > 0:
                            pattern_flag = False
                            break
                    for parent in link.parents:
                        if 'class' in parent.attrs:
                            if 'toccolours' in parent['class']:
                                table_flag = False
                                break
                    if pattern_flag and table_flag:
                        new_page = start_page + url[1:]
                        outgoing_pages.add(new_page)
                        if new_page not in watched_pages and new_page not in new_pages:
                            new_pages.add(new_page)
                            counter += 1
            watched_pages.add(current_page)
            if len(watched_pages) % 100 == 0:
                end_time = time.time()
                print(len(watched_pages), end_time - start_time, len(new_pages), counter)
                start_time = end_time
            out_file.write(parse.unquote(current_page) + '\n')
            for out in outgoing_pages:
                out_file.write("  " + parse.unquote(out) + '\n')

if __name__ == "__main__":
    print('{a}:{b}'.format(a=3, b=4))
    #construct_graph('http://lurkmore.co/', name_of_file='new_result.txt')

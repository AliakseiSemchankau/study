from bs4 import BeautifulSoup
from urllib import request, parse
import time
import pickle


def check_url(url, start_url):
    try:
        req = request.urlopen(url)
    except:
        print(url)
        return []
    soup = BeautifulSoup(req, 'lxml')
    out_urls = []
    for link in soup.find_all('link'):
        rel = link.get('rel')
        if rel == ['canonical']:
            cur_url = start_url + link.get('href')
            out_urls.append(parse.unquote(cur_url))
            break
    return out_urls


def remove_redirections(start_url, name_of_file):
    redir_urls = {}
    checked_urls = set()
    start_time = time.time()
    with open(name_of_file) as res_file:
        with open('redirections_report.txt', 'a') as redir_file:
            with open('redirections_pickle.txt', 'wb') as red_file:
                counter = 0
                for line in res_file.readlines():
                    cur_url = line.strip()
                    if cur_url not in redir_urls and cur_url not in checked_urls:
                        rel = cur_url.split('http://')[1]
                        temp = check_url('http://' + parse.quote(rel), start_url)
                        if len(temp) > 0:
                            if temp[0] in redir_urls:
                                if cur_url not in redir_urls[temp[0]]:
                                    redir_urls[temp[0]].add(cur_url)
                                    redir_file.write(temp[0] + ' --- ' + cur_url + '\n')
                            else:
                                redir_set = set()
                                redir_set.add(cur_url)
                                redir_urls[temp[0]] = redir_set
                                redir_file.write(temp[0] + ' --- ' + cur_url + '\n')
                                checked_urls.add(temp[0])
                        checked_urls.add(cur_url)
                    counter += 1
                    if counter % 100 == 0:
                        print(counter)
                        print(time.time() - start_time)
                        start_time = time.time()
                pickle.dump(redir_urls, red_file)

if __name__ == "__main__":
    remove_redirections('http://lurkmore.co', 'new_result.txt')

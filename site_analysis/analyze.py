import pickle
import copy
from matplotlib import pyplot
import math


class Analyzer(object):
    def __init__(self, graph_filename, redirections_filename, start_page):
        self.graph_filename = graph_filename
        self.redirections_filename = redirections_filename
        self.graph = None
        self.start_page = start_page

    def convert_to_graph(self):
        with open(self.graph_filename) as graph_file:
            self.graph = {}
            source_url = graph_file.readline().strip()
            outgoiing_urls = set()
            for line in graph_file.readlines():
                if line.startswith('  '):
                    outgoiing_urls.add(line.strip())
                else:
                    self.graph[source_url] = copy.deepcopy(outgoiing_urls)
                    outgoiing_urls.clear()
                    source_url = line.strip()

    def get_redirections(self):
        redirections_dict = {}
        with open(self.redirections_filename, 'rb') as redir_file:
            redir_urls = pickle.load(redir_file, encoding='utf-8')
            for key in redir_urls:
                for redir_url in redir_urls[key]:
                    redirections_dict[redir_url] = key
        return redirections_dict

    def get_graph_without_redirections(self):
        redirections_dict = self.get_redirections()
        self.convert_to_graph()
        bad_pages = list()
        for page, outgoing_pages in self.graph.items():
            if page in redirections_dict:
                bad_pages.append(page)
            else:
                outgoing_pages_copy = copy.deepcopy(outgoing_pages)
                for outgoing_page in outgoing_pages:
                    if outgoing_page in redirections_dict:
                        true_page = redirections_dict[outgoing_page]
                        outgoing_pages_copy.add(true_page)
                        outgoing_pages_copy.remove(outgoing_page)
                if page in outgoing_pages:
                    outgoing_pages_copy.remove(page)
                self.graph[page] = outgoing_pages_copy
        for page in bad_pages:
            self.graph.pop(page)

    def delete_bad_urls(self):
        for page, outgoing_pages in self.graph.items():
            bad_pages = []
            for out_page in outgoing_pages:
                if out_page not in self.graph or out_page == self.start_page:
                    bad_pages.append(out_page)
            for bad_page in bad_pages:
                outgoing_pages.remove(bad_page)
        self.graph.pop(self.start_page)

    def print_incoming_degrees(self):
        degrees = {}
        for page, outgoing_pages in self.graph.items():
            start = len(degrees)
            for outgoing_page in outgoing_pages:
                if outgoing_page in degrees:
                    degrees[outgoing_page] += 1
                else:
                    degrees[outgoing_page] = 1

        with open('degrees.txt', 'w') as pr_file:
            for key, value in degrees.items():
                pr_file.write(str(key) + ' --- ' + str(value) + '\n')

        sorted_pages = sorted(degrees, key=lambda x: -degrees[x])
        for page in sorted_pages:
            print(page, degrees[page])

    def calculate_page_rank(self, num_iterations=100, damp_factor=0.8):
        current_page_rank = {}
        previous_page_rank = {}
        n = len(self.graph)
        for page in self.graph:
            previous_page_rank[page] = 1.0 / n
            current_page_rank[page] = 1.0 / n
        for i in range(num_iterations):
            for page, outgoing_pages in self.graph.items():
                for out_page in outgoing_pages:
                    prev_value = previous_page_rank[page]
                    current_page_rank[out_page] += damp_factor * prev_value / len(outgoing_pages)

            for page in self.graph:
                factor = (1 - damp_factor - 0.05)
                current_page_rank[page] += factor * previous_page_rank[page] + 0.05 / n
                previous_page_rank[page] = current_page_rank[page]
                current_page_rank[page] = 0

        return previous_page_rank


def draw_hist(degrees_filename):
    urls = []
    degrees = []
    with open(degrees_filename) as deg_file:
        for line in deg_file.readlines():
            temp = line.split()
            urls.append(temp[0].strip())
            degrees.append((float(temp[1].strip())))

    x = range(len(urls))

    fig = pyplot.figure(figsize=(20,15))
    pyplot.plot(x, degrees)
    #pyplot.xlim(0, 1000)
    pyplot.show()


if __name__ == '__main__':
    example = Analyzer('new_result.txt', 'redirections_pickle.txt',
                       'http://lurkmore.co/')
    example.get_graph_without_redirections()
    example.delete_bad_urls()
    example.print_incoming_degrees()
    pagerank = example.calculate_page_rank()
    sorted_pr = sorted(pagerank, key=lambda x: -pagerank[x])
    for key in sorted_pr:
        print(key, pagerank[key])

    #draw_hist('degrees.txt')

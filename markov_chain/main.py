import re
import random
import argparse
import unittest
import sys


class State(object):
    def __init__(self, token=''):
        self.token = token
        self.frequencies_out = {}
        self.counter = 0

    def add_token(self, new_state):
        flag = False
        for out_state in self.frequencies_out:
            if out_state == new_state:
                flag = True
                self.frequencies_out[out_state] += 1
                self.counter += 1
                break
        if not flag:
            self.frequencies_out[new_state] = 1
            self.counter += 1

    def prob_out(self):
        prob = {}
        for state, frequency in self.frequencies_out.items():
            prob[state.token] = frequency / self.counter
        return prob


def tokenize(line, include_whitespace=False):
    word_pattern = '[А-Яа-яA-Za-z]+'
    number_pattern = '\d+'
    symbol_pattern = '[,:;?!.]'

    if include_whitespace:
        symbol_pattern = '[,:;?!. ]'
    word_iterator = re.finditer(word_pattern, line)
    number_iterator = re.finditer(number_pattern, line)
    symbol_iterator = re.finditer(symbol_pattern, line)

    tokens = []
    iterators = []
    word_tokens = []

    for i in word_iterator:
        word_tokens.append(i.group())
        iterators.append(i)
    for i in number_iterator:
        iterators.append(i)
    for i in symbol_iterator:
        iterators.append(i)
    iterators = sorted(iterators, key=lambda x: x.start())
    for i in iterators:
        tokens.append(i.group())

    return word_tokens, tokens


def pretty_print(sentence, end):
    print(re.sub("\s(?=[,;:?!.])", "", " ".join(sentence)), end=end)


class MarkovChain(object):
    def __init__(self, text, separator_for_sentences=('\n',)):
        empty_state = State('')
        self.state_by_token = {'': empty_state}
        self.tokens = []
        sentences = []
        start_pos = 0
        tokenize_arg = 1

        for i in range(len(text)):
            char = text[i]
            for separator in separator_for_sentences:
                if char == separator:
                    sentences.append(text[start_pos:i + 1])
                    start_pos = i + 1
        sentences.append(text[start_pos:])

        if separator_for_sentences == ['\n']:
            tokenize_arg = 0

        for sentence in sentences:
            sentence = sentence.strip()
            sentence_tokens = tokenize(sentence)[tokenize_arg]
            self.tokens.append(sentence_tokens)
            prev_state = empty_state
            for i in range(len(sentence_tokens)):
                if i > 0:
                    prev_state = self.state_by_token[sentence_tokens[i - 1]]
                current_token = sentence_tokens[i]
                if current_token not in self.state_by_token:
                    current_state = State(current_token)
                    self.state_by_token[current_token] = current_state
                else:
                    current_state = self.state_by_token[current_token]
                prev_state.add_token(current_state)
                if i > 0:
                    empty_state.add_token(current_state)

    def calculate_prob(self, depth):
        for d in range(1, depth + 1):
            for line_tokens in self.tokens:
                for i in range(d - 1, len(line_tokens)):
                    current_token = ' '.join(line_tokens[i - d + 1:i + 1])
                    if current_token not in self.state_by_token:
                        current_state = State(current_token)
                        self.state_by_token[current_token] = current_state
                    else:
                        current_state = self.state_by_token[current_token]
                    if i < len(line_tokens) - 1:
                        current_state.add_token(self.state_by_token[line_tokens[i + 1]])

    def prob_to_string(self):
        res = ''
        for token, state in sorted(self.state_by_token.items(), key=lambda x: x[0]):
            prob_out = state.prob_out()
            if len(prob_out) > 0:
                res += token + '\n'
                for next_token, prob in sorted(state.prob_out().items(), key=lambda x: x[0]):
                    res += ('  {0}: {1:.2f}'.format(next_token, prob)) + '\n'
        return res

    def get_next_generated_token(self, cur_token):
        symbol_pattern = '[,:;?!. ]'
        outgoing_tokens = self.state_by_token[' '.join(cur_token)]
        prob_out = outgoing_tokens.prob_out()
        prob_out_tokens = []
        prob_out_values = []
        for token in prob_out:
            prob_out_tokens.append(token)
            prob_out_values.append(prob_out[token])
        start_sentence = False
        if len(cur_token) == 0:
            start_sentence = True
        while True:
            rand_number = random.random()
            prev_value = 0
            value = 0
            for i in range(len(prob_out_values)):
                prev_value = value
                value += prob_out_values[i]
                if prev_value <= rand_number < value:
                    next_token = prob_out_tokens[i]
                    break
            if len(re.findall(symbol_pattern, next_token)) == 0:
                start_sentence = False
            if not start_sentence:
                break
        return next_token

    def print_generated_text(self, depth=1, size=100):
        number_of_words = 0
        max_number_of_words = size
        whole_cur_sentence = []
        cur_part_sentence = []
        while number_of_words < max_number_of_words:
            next_token = self.get_next_generated_token(cur_part_sentence)
            depth_flag = True
            if next_token == '.' or next_token == '?' or next_token == '!':
                whole_cur_sentence[0] = whole_cur_sentence[0].capitalize()
                pretty_print(whole_cur_sentence + [next_token], end='\n')
                number_of_words += len(cur_part_sentence) + 1
                cur_part_sentence = []
                whole_cur_sentence = []
                depth_flag = False
            if len(cur_part_sentence) == depth:
                number_of_words += 1
                cur_part_sentence.pop(0)
                whole_cur_sentence.append(next_token)
                cur_part_sentence.append(next_token)
                depth_flag = False
            elif depth_flag:
                whole_cur_sentence.append(next_token)
                cur_part_sentence.append(next_token)


def test():
    class TestMarkovChain(unittest.TestCase):
        def test_tokenize(self):
            line = 'Hello, world!'
            self.assertEqual('\n'.join(tokenize(line, include_whitespace=True)[1]),
                             'Hello\n,\n \nworld\n!')

        def test_prob(self):
            lines = 'First test\nSecond test'
            ex = MarkovChain(lines)
            ex.calculate_prob(depth=1)
            result = '\n  First: 0.25\n  Second: 0.25\n  test: 0.50\nFirst\n'
            result += '  test: 1.00\nSecond\n  test: 1.00\n'
            self.assertEqual(ex.prob_to_string(), result)

    runner = TestMarkovChain()
    runner.test_tokenize()
    runner.test_prob()


def main():
    with open('input.txt') as input_file:
        text_args = input_file.readline().split()
        text = ''.join(input_file.readlines())
        text = text.strip()

    parser = argparse.ArgumentParser()

    subparsers = parser.add_subparsers(dest='command')

    parser_tokenize = subparsers.add_parser('tokenize', help='tokenize text')
    parser_tokenize.set_defaults(func=tokenize)

    parser_probs = subparsers.add_parser('probabilities', help='show probabilities')
    parser_probs.add_argument('--depth', type=int, default=1)

    parser_generate = subparsers.add_parser('generate', help='generate sequence')
    parser_generate.add_argument('--depth', type=int, default=1)
    parser_generate.add_argument('--size', type=int, default=10)

    parser_unit_test = subparsers.add_parser('test', help='for unit test')

    args = parser.parse_args(text_args)

    if args.command == 'tokenize':
        tokens = tokenize(text, include_whitespace=True)[1]
        for token in tokens:
            print(token)
    if args.command == 'probabilities':
        ex = MarkovChain(text, separator_for_sentences=['\n'])
        ex.calculate_prob(depth=args.depth)
        print(ex.prob_to_string(), end='')
    if args.command == 'generate':
        ex = MarkovChain(text, separator_for_sentences=['!', '.', '?'])
        ex.calculate_prob(depth=args.depth)
        ex.print_generated_text(depth=args.depth, size=args.size)
    if args.command == 'test':
        test()


if __name__ == '__main__':
    main()

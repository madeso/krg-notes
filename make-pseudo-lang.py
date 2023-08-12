#!/usr/bin/env python3

# pseudo language inspired by campo santo https://youtu.be/wj-2vbiyHnI?t=2444
# useful for testing languages

import argparse
from enum import StrEnum, auto

class Language(StrEnum):
    METAL = auto()
    LONG = auto()
    ROBBER = auto()


def make_pseudo_char(lang: Language, c: str) -> str:
    match lang:
        case Language.METAL:
            match c:
                case 'I': return 'Î'
                case 'E': return 'Ê'
                case 'S': return 'Š'
                case 'U': return 'Ü'
                case 'C': return 'Ç'
                case 'O': return 'Ö'
                case 'N': return 'Ñ'
                case 'L': return 'Ḷ'
        
                case 'i': return 'î'
                case 'e': return 'ê'
                case 's': return 'š'
                case 'u': return 'ü'
                case 'c': return 'ç'
                case 'o': return 'ö'
                case 'n': return 'ñ'
                case 'l': return 'ḷ'
                case _:
                    return c
        case Language.LONG:
            if c.lower() in 'aoueiy':
                return c + c.lower()
            else:
                return c
        case Language.ROBBER:
            tr = lambda x: x[0].upper() + x[1:] if c != c.lower() else x
            match c.lower():
                case 'q': return tr('qaq')
                case 'w': return tr('wow')
                case 'r': return tr('reh')
                case 't': return tr('tut')
                case 'p': return tr('pap')
                case 's': return tr('sus')
                case 'd': return tr('ded')
                case 'f': return tr('faf')
                case 'g': return tr('gog')
                case 'h': return tr('heh')
                case 'j': return tr('joj')
                case 'k': return tr('kek')
                case 'l': return tr('lol')
                case 'z': return tr('ziz')
                case 'x': return tr('xux')
                case 'c': return tr('cac')
                case 'v': return tr('vev')
                case 'b': return tr('bob')
                case 'n': return tr('non')
                case 'm': return tr('mem')
                case _:
                    return c
        case _:
            raise "invalid case"


def make_pseudo(lang: Language, input: str) -> str:
    return ''.join(make_pseudo_char(lang, c) for c in input)


def main():
    parser = argparse.ArgumentParser(prog='make-pseudo-lang', description="pseudo languages converter that makes english more metal or longer")
    parser.add_argument('input', help="the input string")
    parser.add_argument('--lang', choices=[str(l) for l in Language], default=str(Language.METAL), help="change the language used")

    args = parser.parse_args()
    print(make_pseudo(Language(args.lang), args.input))


if __name__ == "__main__":
    main()


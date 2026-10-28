"""Repository checks for Angry Pig: relative links and images resolve, and no build-machine paths leak.

Run from the repository root: python .github/scripts/check_repo.py
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
LINK = re.compile(r'\]\(([^)\s]+)(?:\s+"[^"]*")?\)')
FORBIDDEN = [
    (re.compile(r'[A-Za-z]:\\\\Users', re.IGNORECASE), 'Windows user path'),
    (re.compile(r'\bD:[\\/]'), 'developer drive path'),
    (re.compile(r'AP-RE'), 'internal project folder name'),
    (re.compile(r'sausage|fattener', re.IGNORECASE), 'third-party product reference'),
    (re.compile(r'gh[pousr]_[A-Za-z0-9]{20,}'), 'GitHub token pattern'),
]
TEXT_EXT = ('.md', '.yml', '.yaml', '.txt', '.ps1', '.py')


def files():
    for dirpath, dirnames, filenames in os.walk(ROOT):
        dirnames[:] = [d for d in dirnames if d != '.git']
        for name in filenames:
            yield os.path.join(dirpath, name)


def main():
    problems = []
    for path in files():
        rel = os.path.relpath(path, ROOT)
        if path.endswith('check_repo.py'):
            continue
        if path.endswith(TEXT_EXT):
            with open(path, encoding='utf-8', errors='replace') as f:
                text = f.read()
            for pattern, label in FORBIDDEN:
                for m in pattern.finditer(text):
                    line = text.count('\n', 0, m.start()) + 1
                    problems.append(f'{rel}:{line}: {label}')
        if path.endswith('.md'):
            with open(path, encoding='utf-8', errors='replace') as f:
                text = f.read()
            for m in LINK.finditer(text):
                target = m.group(1)
                if target.startswith(('http://', 'https://', 'mailto:', '#')):
                    continue
                target = target.split('#', 1)[0]
                if not target:
                    continue
                resolved = os.path.normpath(os.path.join(os.path.dirname(path), target))
                if not os.path.exists(resolved):
                    line = text.count('\n', 0, m.start()) + 1
                    problems.append(f'{rel}:{line}: broken relative link -> {target}')
    for p in problems:
        print(p)
    print(f'{len(problems)} problem(s) found')
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())

#!/usr/bin/env python3
"""Build and exercise the database contract through Builder and its published header."""
from pathlib import Path
import datetime
import hashlib
import json
import os
import shlex
import subprocess
import sys

MODULE = 'm03h5hm86mnvc18opf1rdttsgh_database'
root = Path(sys.argv[1] if len(sys.argv) == 2 else os.environ.get('BUILDER_WORKSPACE_ROOT', Path.cwd())).resolve()
artifacts = Path(os.environ.get('BUILDER_ARTIFACT_ROOT', root / 'artifacts')).resolve()
module = root / 'ws2' / MODULE
if not (root / 'ws0').is_dir() or not module.is_dir():
    raise SystemExit('Run from Builder-Layout, or pass its root as the only argument.')
run = artifacts / 'feasibility' / 'database-table-refactor' / datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ')
run.mkdir(parents=True)
compiler = os.environ.get('CXX', '/usr/bin/g++')
environment = dict(os.environ, BUILDER_WORKSPACE_ROOT=str(root), BUILDER_ARTIFACT_ROOT=str(artifacts))
environment.setdefault('CCACHE_DIR', str(artifacts / 'feasibility' / 'ccache'))
commands = []
passed = []


def execute(name, command, expected_failure=False):
    commands.append(shlex.join(command))
    with (run / f'{name}.log').open('w') as output:
        result = subprocess.run(command, env=environment, stdout=output, stderr=subprocess.STDOUT)
    diagnostics = (run / f'{name}.log').read_text()
    if (result.returncode == 0) == expected_failure:
        print(diagnostics[-14000:])
        raise RuntimeError(f'{name}: unexpected exit {result.returncode}; logs: {run}')
    return diagnostics


version = execute('compiler', [compiler, '--version']).splitlines()[0]
driver = run / 'build_library'
command = [compiler, '-std=c++26', '-freflection', f'-I{root}/ws0', str(module / 'test/build_library.cpp'), '-Wl,--no-as-needed']
# Foundation libraries are linked as a group because their SCC libraries leave peer symbols unresolved.
for dependency in sorted((root / 'ws0').iterdir()):
    install = (artifacts / dependency.name / 'latest/library/install').resolve()
    library = install / f'lib{dependency.name}.so'
    if library.is_file():
        command += [str(library), f'-Wl,-rpath,{install}']
command += ['-o', str(driver)]
execute('build_driver', command)
print('Running Builder source/interface/library pipeline and registered validations...', flush=True)
execute('pipeline', [str(driver)])
passed.append('Builder pipeline: transactions, private backend integration, standalone header, README example, external client codecs, and public API validations')
print('PASS Builder pipeline and six registered validations', flush=True)

interface = (artifacts / MODULE / 'latest/interface/install').resolve(strict=True)
library = (artifacts / MODULE / 'latest/library').resolve(strict=True)
assert (library / '.complete').is_file()
assert sorted(str(path.relative_to(interface)) for path in interface.rglob('*.h')) == [f'{MODULE}/database.h']
assert (interface / MODULE / 'database.h').read_bytes() == (module / 'database.h').read_bytes()
readme = (module / 'README.md').read_text().split('```cpp\n', 1)[1].split('```', 1)[0]
assert readme.strip() == (module / 'test/readme_example.cpp').read_text().strip()
passed += ['Only database.h is published; source and published header match', 'README example matches the compiled validation source exactly']

dependency_interfaces = {
    dependency: (artifacts / dependency / 'latest/interface/install').resolve(strict=True)
    for dependency in ['m03gagbht2l61mj6qitacwbmea_byte_stream', 'm03gagbhsnusi43zogoacgj2ez_filesystem', 'm03h5f8oo5d48avkx9mdk2bi5u_sqlite']
}
flags = ['-std=c++26', '-freflection', '-Wall', '-Wextra', '-Wpedantic', '-Werror', f'-I{interface}', f'-I{dependency_interfaces["m03gagbht2l61mj6qitacwbmea_byte_stream"]}', '-fsyntax-only']
filesystem_include = f'-I{dependency_interfaces["m03gagbhsnusi43zogoacgj2ez_filesystem"]}'
backend_include = f'-I{dependency_interfaces["m03h5f8oo5d48avkx9mdk2bi5u_sqlite"]}'
for filename in ['test/transactions.cpp', 'test/standalone_header.cpp', 'test/readme_example.cpp', 'test/client_codecs.cpp']:
    execute('warnings_' + Path(filename).stem, [compiler, *flags, str(module / filename)])
execute('warnings_public_api', [compiler, *flags, filesystem_include, str(module / 'test/public_api.cpp')])
for filename in ['database.cpp', 'test/backend.cpp', 'test/faults.cpp']:
    execute('warnings_' + Path(filename).stem, [compiler, *flags, filesystem_include, backend_include, str(module / filename)])
header = (interface / MODULE / 'database.h').read_text()
for forbidden in ['sqlite', 'SQLite', 'rowid', 'STRICT', 'PRAGMA', 'main-schema']:
    assert forbidden not in header, f'Backend detail in published header: {forbidden}'
assert 'sqlite' not in (module / 'README.md').read_text().lower()
passed += ['Consumer checks compile without backend include paths; public header and README contain no backend identity or structure', 'Implementation and validations pass strict compiler warnings']

# Builder supplies the source-derived link closure for its registered client
# executable; the isolated checks above independently prove header requirements.
passed.append('External client codec executable runs through Builder; consumer compilation needs no backend headers')
passed += ['Public transactions: callback identity/count, existing handles, shared snapshots, own changes, atomic visibility, contention, recoverable errors and rollback', 'Typed removal: missing no-op, rollback, compatible reuse and incompatible replacement preservation', 'Private fault injection: implicit/explicit commit failures, failed operation recovery, automatic rollback and original-exception preservation', 'Public literal filename, arbitrary table-name and closed-storage copy regressions']

for fixture in sorted((module / 'test/compile_fail').glob('*.cpp')):
    expected = fixture.read_text().splitlines()[0].removeprefix('// expected: ')
    diagnostics = execute(f'negative_{fixture.stem}', [compiler, *flags, str(fixture)], expected_failure=True)
    assert expected in diagnostics, f'{fixture.name}: expected diagnostic {expected!r}; see {run}'
    passed.append(f'Compile rejection {fixture.stem}: {expected}')
    print(f'PASS expected compile rejection: {fixture.stem}', flush=True)

source_hashes = {str(path.relative_to(module)): hashlib.sha256(path.read_bytes()).hexdigest() for path in sorted(module.rglob('*')) if path.is_file()}
(run / 'manifest.json').write_text(json.dumps({'compiler': version, 'module': MODULE, 'source_sha256': source_hashes, 'library_artifact': str(library), 'interface_artifact': str(interface)}, indent=2) + '\n')
(run / 'commands.txt').write_text('\n'.join(commands) + '\n')
report = ['# Database table API validation', '', version, '', 'Reproduce from Builder-Layout:', '', '```sh', f'python3 ws2/{MODULE}/test/check.py', '```', '', '## Results', '', *[f'- PASS {item}' for item in passed], '', '## Limits', '', 'Custom codec ownership is an author obligation. Native-call fault injection covers commit and rollback failure paths, not every physical I/O failure. Live filesystem copying and externally authored storage layouts remain outside the contract.', '', f'Full commands, outputs, and source hashes: `{run}`.', '']
(run / 'REPORT.md').write_text('\n'.join(report))
print(f'PASS all checks; retained report: {run}/REPORT.md', flush=True)

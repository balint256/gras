# benchmark registry

import os

def tokwargs(**kwargs): return kwargs

INSTALL_PREFIX = '/opt/usr'

DURATION = '1.7' #seconds

GRAS_ENV = {
    'PATH': os.path.join(INSTALL_PREFIX, 'gras/bin:$PATH:%s'%os.getenv('PATH')),
    'LD_LIBRARY_PATH': os.path.join(INSTALL_PREFIX, 'gras/lib:%s'%os.getenv('LD_LIBRARY_PATH')),
    'PYTHONPATH': os.path.join(INSTALL_PREFIX, 'gras/lib/python2.7/dist-packages:%s'%os.getenv('PYTHONPATH')),
}

GR_ENV = {
    'PATH': os.path.join(INSTALL_PREFIX, 'gr/bin:$PATH:%s'%os.getenv('PATH')),
    'LD_LIBRARY_PATH': os.path.join(INSTALL_PREFIX, 'gr/lib:%s'%os.getenv('LD_LIBRARY_PATH')),
    'PYTHONPATH': os.path.join(INSTALL_PREFIX, 'gr/lib/python2.7/dist-packages:%s'%os.getenv('PYTHONPATH')),
}

BENCHMARK_MANY_11_BLOCKS = tokwargs(
    wat='Benchmark the schedulers with many 1:1 ratio blocks',
    moar='''\
- Compare simultaneous 1:1 ratio blocks in each scheduler.
- GRAS will use only the buffer pool allocator,
and every work will fully consume available buffers.''',
    tests = [
        tokwargs(wat='GRAS',     args=['tb_many_1_to_1_blocks.py', DURATION], env=GRAS_ENV),
        tokwargs(wat='GR',       args=['tb_many_1_to_1_blocks.py', DURATION], env=GR_ENV),
    ],
)

BENCHMARK_MANY_RATE_BLOCKS = tokwargs(
    wat='Benchmark the schedulers with many rate changing blocks',
    moar='''\
- Compare simultaneous changing ratio blocks in each scheduler.
- GRAS will use only the buffer pool allocator,
and every work will fully consume available buffers.''',
    tests = [
        tokwargs(wat='GRAS',     args=['tb_many_rate_changes.py', '--dur', DURATION], env=GRAS_ENV),
        tokwargs(wat='GR',       args=['tb_many_rate_changes.py', '--dur', DURATION], env=GR_ENV),
    ],
)

BENCHMARK_FILTER_BLOCK = tokwargs(
    wat='Benchmark the schedulers with a filter block',
    moar='''\
- Compare filter blocks in each scheduler.
- Shows both schedulers using circular buffer.
- The decimating FIR filter is compared.
- The rational resampler filter is compared.''',
    tests = [
        tokwargs(wat='GRAS decim FIR',     args=['tb_filter_block.py', '--dur', DURATION, '--which', 'dfir'], env=GRAS_ENV),
        tokwargs(wat='GR decim FIR',       args=['tb_filter_block.py', '--dur', DURATION, '--which', 'dfir'], env=GR_ENV),
        tokwargs(wat='GRAS resampler',     args=['tb_filter_block.py', '--dur', DURATION, '--which', 'resamp'], env=GRAS_ENV),
        tokwargs(wat='GR resampler',       args=['tb_filter_block.py', '--dur', DURATION, '--which', 'resamp'], env=GR_ENV),
    ],
)

BENCHMARK_MATH_OPS = tokwargs(
    wat='Benchmark GrExtras vs gr-blocks math blocks',
    moar='''\
- Compare math block implementations using GRAS.
- All blocks are using vector optimization.
- GrExtras math blocks avoid an unnecessary memcpy.
- GrExtras math blocks enable automatic bufer in-placing.''',
    tests = [
        tokwargs(wat='GrExtras Add\n(GRAS)',        args=['tb_grextras_math.py', DURATION, 'extras_add'], env=GRAS_ENV),
        tokwargs(wat='gr-blocks Add\n(GRAS)',       args=['tb_grextras_math.py', DURATION, 'blocks_add'], env=GRAS_ENV),
        tokwargs(wat='gr-blocks Add\n(GR)',         args=['tb_grextras_math.py', DURATION, 'blocks_add'], env=GR_ENV),
        tokwargs(wat='GrExtras Mult\n(GRAS)',       args=['tb_grextras_math.py', DURATION, 'extras_mult'], env=GRAS_ENV),
        tokwargs(wat='gr-blocks Mult\n(GRAS)',      args=['tb_grextras_math.py', DURATION, 'blocks_mult'], env=GRAS_ENV),
        tokwargs(wat='gr-blocks Mult\n(GR)',        args=['tb_grextras_math.py', DURATION, 'blocks_mult'], env=GR_ENV),
    ],
)

BENCHMARK_DELAY_BLOCKS = tokwargs(
    wat='Benchmark GrExtras vs gr-core delay block',
    moar='''\
- Compare delay block implementations using GRAS.
- The GrExtras implementation uses zero-copy.''',
    tests = [
        tokwargs(wat='GrExtras Delay\n(GRAS)',          args=['tb_grextras_delay.py', DURATION, 'extras_delay'], env=GRAS_ENV),
        tokwargs(wat='gr-core Delay\n(GRAS)',           args=['tb_grextras_delay.py', DURATION, 'core_delay'], env=GRAS_ENV),
        tokwargs(wat='gr-core Delay\n(GR)',             args=['tb_grextras_delay.py', DURATION, 'core_delay'], env=GR_ENV),
    ],
)

BENCHMARKS = (
    BENCHMARK_MANY_11_BLOCKS,
    BENCHMARK_MANY_RATE_BLOCKS,
    BENCHMARK_FILTER_BLOCK,
    BENCHMARK_MATH_OPS,
    BENCHMARK_DELAY_BLOCKS,
)

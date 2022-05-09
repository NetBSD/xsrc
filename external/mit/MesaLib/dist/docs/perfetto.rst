Perfetto Tracing
================

Mesa has experimental support for `Perfetto <https://perfetto.dev>`__ for
GPU performance monitoring.  Perfetto supports multiple
`producers <https://perfetto.dev/docs/concepts/service-model>`__ each with
one or more data-sources.  Perfetto already provides various producers and
data-sources for things like:

- CPU scheduling events (``linux.ftrace``)
- CPU frequency scaling (``linux.ftrace``)
- System calls (``linux.ftrace``)
- Process memory utilization (``linux.process_stats``)

As well as various domain specific producers.

The mesa perfetto support adds additional producers, to allow for visualizing
GPU performance (frequency, utilization, performance counters, etc) on the
same timeline, to better understand and tune/debug system level performance:

- pps-producer: A systemwide daemon that can collect global performance
  counters.
- mesa: Per-process producer within mesa to capture render-stage traces
  on the GPU timeline, track events, etc.

The exact supported features vary per driver:

.. list-table:: Supported data-sources
   :header-rows: 1

   * - Driver
     - PPS Counters
     - Render Stages
   * - Freedreno
     - ``gpu.counters.msm``
     - ``gpu.renderstages.msm``
   * - Turnip
     - ``gpu.counters.msm``
     -
   * - Intel
     - ``gpu.counters.i915``
     -
   * - Panfrost
     - ``gpu.counters.panfrost``
     -

Run
---

To capture a trace with perfetto you need to take the following steps:

1. Build perfetto from sources available at ``subprojects/perfetto`` following
   `this guide <https://perfetto.dev/docs/quickstart/linux-tracing>`__.

2. Create a `trace config <https://perfetto.dev/#/trace-config.md>`__, which is
   a json formatted text file with extension ``.cfg``, or use one of the config
   files under the ``src/tool/pps/cfg`` directory. More examples of config files
   can be found in ``subprojects/perfetto/test/configs``.

3. Change directory to ``subprojects/perfetto`` and run a
   `convenience script <https://perfetto.dev/#/running.md>`__ to start the
   tracing service:

   .. code-block:: console

      cd subprojects/perfetto
      CONFIG=<path/to/gpu.cfg> OUT=out/linux_clang_release ./tools/tmux -n

4. Start other producers you may need, e.g. ``pps-producer``.

5. Start ``perfetto`` under the tmux session initiated in step 3.

6. Once tracing has finished, you can detach from tmux with :kbd:`Ctrl+b`,
   :kbd:`d`, and the convenience script should automatically copy the trace
   files into ``$HOME/Downloads``.

7. Go to `ui.perfetto.dev <https://ui.perfetto.dev>`__ and upload
   ``$HOME/Downloads/trace.protobuf`` by clicking on **Open trace file**.

8. Alternatively you can open the trace in `AGI <https://gpuinspector.dev/>`__
   (which despite the name can be used to view non-android traces).

Driver Specifics
~~~~~~~~~~~~~~~~

Below is driver specific information/instructions for the PPS producer.

Freedreno / Turnip
^^^^^^^^^^^^^^^^^^

The Freedreno PPS driver needs root access to read system-wide
performance counters, so you can simply run it with sudo:

.. code-block:: console

   sudo ./build/src/tool/pps/pps-producer

Intel
^^^^^

The Intel PPS driver needs root access to read system-wide
`RenderBasic <https://software.intel.com/content/www/us/en/develop/documentation/vtune-help/top/reference/gpu-metrics-reference.html>`__
performance counters, so you can simply run it with sudo:

.. code-block:: console

   sudo ./build/src/tool/pps/pps-producer

Another option to enable access wide data without root permissions would be running the following:

.. code-block:: console

   sudo sysctl dev.i915.perf_stream_paranoid=0

Alternatively using the ``CAP_PERFMON`` permission on the binary should work too.

Panfrost
^^^^^^^^

The Panfrost PPS driver uses unstable ioctls that behave correctly on
kernel version `5.4.23+ <https://lwn.net/Articles/813601/>`__ and
`5.5.7+ <https://lwn.net/Articles/813600/>`__.

To run the producer, follow these two simple steps:

1. Enable Panfrost unstable ioctls via kernel parameter:

   .. code-block:: console

      modprobe panfrost unstable_ioctls=1

   Alternatively you could add ``panfrost.unstable_ioctls=1`` to your kernel command line, or ``echo 1 > /sys/module/panfrost/parameters/unstable_ioctls``.

2. Run the producer:

   .. code-block:: console

      ./build/pps-producer

Troubleshooting
---------------

Tmux
~~~~

If the convenience script ``tools/tmux`` keeps copying artifacts to your
``SSH_TARGET`` without starting the tmux session, make sure you have ``tmux``
installed in your system.

.. code-block:: console

   apt install tmux

Missing counter names
~~~~~~~~~~~~~~~~~~~~~

If the trace viewer shows a list of counters with a description like
``gpu_counter(#)`` instead of their proper names, maybe you had a data loss due
to the trace buffer being full and wrapped.

In order to prevent this loss of data you can tweak the trace config file in
two different ways:

- Increase the size of the buffer in use:

  .. code-block:: javascript

      buffers {
          size_kb: 2048,
          fill_policy: RING_BUFFER,
      }

- Periodically flush the trace buffer into the output file:

  .. code-block:: javascript

      write_into_file: true
      file_write_period_ms: 250


- Discard new traces when the buffer fills:

  .. code-block:: javascript

      buffers {
          size_kb: 2048,
          fill_policy: DISCARD,
      }

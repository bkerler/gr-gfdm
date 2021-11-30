GNU Radio GFDM Modulator/Demodulator
================

The *gr-gfdm* project is a Free Software Package which aims to provide an implementation of Generalized Frequency Division Multiplexing (GFDM) in the GNU Radio framework. GFDM is a proposed waveform for use in 5G.

This project was initiated as a Bachelor thesis at the *Communication Engineering Lab (CEL)* at the *Karlsruhe Institute of Technology (KIT)*, Germany, <http://www.cel.kit.edu>.
It has since evolved and is used in over-the-air demos at the [*Department of Communications Engineering (ANT)*](https://www.ant.uni-bremen.de) at [*University of Bremen*](https://uni-bremen.de).

Concept
-------------
Tackling current issues with *OFDM* several new waveforms are proposed for consideration in 5G. *FBMC*, *UFMC*, *BFDM* and *GFDM* are the names of the proposed waveforms and they are filtered multicarrier systems. Orthogonality of neighboring carriers is no constraint.

*GFDM* was proposed by the Vodafone Chair of TU Dresden and accurately described in [1]. Due to the high complexity of a Transmit-Matrix-based approach a low coplexity receiver [2] and transmitter [3] are proposed.

*GFDM* can be described by parallelizing several *SC-FDE* streams on subcarrier. The transmit symbols are localized in a time/frequency-grid and pulshaping is applied subcarrier-wise. After pulshaping and localizing on the correct subcarrier-frequency the symbolstreams are superpositioned and can be transmitted. On receiver side the symbols on the subcarrier can be extracted by applying a *MF*, *ZF* or *MMSE*-filter of the previous pulseshaping filter. Non-orthogonality of neighboring subcarrier introduces *ICI* if demodulating with *MF*. A successive interference cancellation algorithm is proposed to remove interference.

Due to its block-nature a block synchronisation with improved Schmidl & Cox - Symbols can be achievd.

Capabilities
-------------

For generating a transmit symbol stream *gr-gfdm* provides following blocks:

- resource mapper
- modulator
- cyclic prefixer

For reception of a GFDM symbol stream *gr-gfdm* provides following blocks:

- channel estimator
- prefix remover
- simple demodulator
- advanced demodulator with IC
- demapper

Signal processing in the blocks is performed in *kernels* which have no dependency on *GNU Radio*. Thus inclusion in interfaces to other programming languages is easier.

The modulator and demodulator are implemented using the low complexity approach with Sparse Frequency Domain processing and heavy use of *FFTW* and *VOLK* to accelerate signal processing on modern *GPP*s.

Tests of optimized C++ code are performed with a reference implementation in Python.
Additionally the C++ code is wrapped with [PyBind11](https://github.com/pybind/pybind11).

Synchronization algorithms are not part of this OOT. Instead we rely on [`XFDMSync`](https://github.com/jdemel/XFDMSync). It is a better optimized multicarrier synchronization OOT.

Requirements
------------
- GNU Radio OOT
  - GNU Radio 3.9 (verified)
  - GR 3.9 API
  - GR-FFT
  - GR-FILTER
  - VOLK
  - UHD (examples)
  - [XFDMSync](https://github.com/jdemel/XFDMSync) (synchronization)

- PyGFDM
  - Numpy
  - Scipy
  - Commpy (`pip3 install scikit-commpy`)
  - Matplotlib

- Conda
  - pyind11
  - fmt
If you want to install gr-gfdm within a Conda installed GNU Radio, these packages need to be installed.
`conda install -c conda-forge pybind11 fmt`.

Build/Install instructions
------------------------------------

1. Install/Build GNU Radio with at least support for FFT,FILTER and VOLK

2. Get *gr-gfdm* from github - `git clone https://github.com/jdemel/gr-gfdm.git`

3. Configure *gr-gfdm* - `mkdir build && cd build && cmake ../`

4. Build and install *gr-gfdm* - `make && sudo make install` (default install target: /usr/local)

5. Configure custom blocks path in GNU Radio Companion to use `/usr/local/share/gnuradio/grc/blocks`

Troubleshooting/Bugs
------------------------------------

In case you encounter bugs not related to *GNU Radio* core functions in *gr-gfdm* open an issue at <https://github.com/jdemel/gr-gfdm/issues>.
Otherwise consider reporting the issue to the GNU Radio project <https://www.gnuradio.org>.

Code formatting
--------
We follow the GNU Radio coding guidelines. This is the current way to format everything:

`find . -regex '.*\.\(c\|cc\|cpp\|cxx\|h\|hh\)' -not -path "*build*" -exec clang-format -style=file -i {} \;`

Further, we have a GitHub Action to check proper code formatting.

References
-------------
1. N. Michailow et al. “Generalized Frequency Division Multiplexing for 5th Generation Cellular Networks”. In: Communications, IEEE Transactions on 62.9 (2014), S. 3045–3061. doi: 10.1109/TCOMM.2014.2345566.

2. I.S. Gaspar et al. “Low Complexity GFDM Receiver Based on Sparse Frequency Domain Processing”. In: Vehicular Technology Conference (VTC Spring), 2013 IEEE 77th. IEEE, 2013, S. 1–6. doi: 10.1109/VTCSpring.2013.6692619.

3. N. Michailow et al. “Generalized frequency division multiplexing: Analysis of an alternative multi-carrier technique for next generation cellular systems”. In: Wireless Communication Systems (ISWCS), 2012 International Symposium on. IEEE, 2012, S. 171–175. doi: 10.1109/ISWCS.2012.6328352.

4. J. Demel, C. Bockelmann, A. Dekorsy "Evaluation of a Software Defined GFDM Implementation for Industry 4.0 Applications". In: IEEE International Conference on Industrial Technology (ICIT 2017), Toronto, Canada, 22. - 25. March 2017

5. J. Demel, C. Bockelmann, A. Dekorsy, Andrej Rode, Sebastian Koslowski, Friedrich K. Jondral "An optimized GFDM software implementation for future Cloud-RAN and field tests". In: GNU Radio Conference 2017, San Diego, USA, 11. - 15. September 2017

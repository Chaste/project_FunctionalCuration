"""Copyright (c) 2005-2015, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

import inspect

class ProtocolError(Exception):
    """Main class for errors raised by the functional curation framework that are intended for user viewing."""
    def __init__(self, *msgParts):
        """Create a protocol error message.
        
        The arguments are joined to create the message string as for the print: converted to strings and space separated.
        In addition, when the exception is created the stack will be examined to determine what lines in the currently
        running protocol were responsible, if any, and these details added to the Python stack trace reported.
        """
        # Figure out where in the protocol this error arose
        locations = self.locations = []
        for frame in reversed(inspect.stack()):
            local_vars = frame[0].f_locals
            obj = local_vars.get('self', None)
            if obj and hasattr(obj, 'location') and (not locations or obj.location != locations[-1]):
                    locations.append(obj.location)
        # Construct the full error message
        msg = ' '.join(map(str, msgParts))
        if locations:
            msg = msg + '\nProtocol stack trace (most recent call last):\n' + '\n'.join(locations)
        super(ProtocolError, self).__init__(msg)

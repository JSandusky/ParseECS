using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ParMake
{
    /// Not actually much of a 'Make' program
    /// This is done because no existing make program is well designed for CLEANLY and RELAIBLY interfacing a larger project into a smaller one
        /// Visual Studio projects are generated
    /// Actual task is pretty much just running cl.exe with various parameters using files in specific locations or lists of files
    /// This facilitates the live build in which the user DLL can be rebuild
    /// It also facilitates final package deployment
    class Program
    {
        static void Main(string[] args)
        {
        }
    }
}

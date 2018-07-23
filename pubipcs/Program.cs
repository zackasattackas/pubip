using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace pubipcs
{
    class Program
    {
        static void Main(string[] args)
        {
            var request = WebRequest.CreateHttp("https://api.ipify.org");

            using (var response = request.GetResponse())
            using (var reader = new StreamReader(response.GetResponseStream() ?? throw new Exception()))
                Console.Write(reader.ReadToEnd());
            
        }
    }
}

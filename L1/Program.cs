using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace L1
{
    struct Row
    {
        public char letter;

        public int count;

        public double probability;
    }
    class Task
    {

        int m;
        int n;
        int b;
        Row[] table;

        Task(string text, int b)
        {
        }

        double entropy()
        {
            double e = 0;

            Row a;

            foreach(Row row in table)
            {
                e += row.probability + Math.Log(row.probability, b);
            }
            
            return e;
        }

        double maxEntropy()
        {
            return;
        }

        double quontity()
        {
            return;
        }

        double absoluteRedundancy()
        {
            return;
        }

        double 
    }

    class Program
    {
        static void Main(string[] args)
        {

        }
    }
}

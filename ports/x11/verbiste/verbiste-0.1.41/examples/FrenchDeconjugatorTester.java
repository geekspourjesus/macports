/*  $Id: FrenchDeconjugatorTester.java,v 1.2 2012/11/18 19:56:04 sarrazip Exp $
    FrenchDeconjugatorTester.java - Test for the french-deconjugator command

    verbiste - French conjugation system
    Copyright (C) 2003 Pierre Sarrazin <http://sarrazip.com/>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

import java.io.*;


public class FrenchDeconjugatorTester
{
    public static void main(String[] args)
    {
	String cmd = "french-deconjugator";

	if (args.length > 0)
	    cmd = args[0];

	try
	{
	    Process process = Runtime.getRuntime().exec(cmd);

	    OutputStreamWriter w = new OutputStreamWriter(
						process.getOutputStream());
	    BufferedReader r = new BufferedReader(
			    new InputStreamReader(process.getInputStream()));

	    BufferedReader in = new BufferedReader(
					new InputStreamReader(System.in));
	    String inputLine;

	    while ((inputLine = in.readLine()) != null)
	    {
		inputLine.trim();
		w.write(inputLine + "\n");
		w.flush();

		String received;
		while ((received = r.readLine()) != null)
		{
		    if (received.length() == 0)
			break;
		    System.out.println("\t" + received);
		}
	    }
	    w.close();
	    r.close();
	    process.destroy();
	}
	catch (IOException e)
	{
	    System.out.println(cmd + ": " + e.getMessage());
	    System.exit(1);
	}
	catch (Exception e)
	{
	    System.out.println("Error: " + e.getMessage());
	    System.exit(1);
	}
    }
}

% verbiste.cls
%
% This class is for the output from verbiste, the
% French verb conjugator by Pierre Sarrazin
%
%   http://sarrazip.com/dev/verbiste.html
%
% This file was written by J.J. Green, 2009 and is
% hereby contributed to the public domain

\ProvidesClass{verbiste}[2009/06/30]

% commands for the cfg file 

\newif\ifvlines \vlinesfalse
\newif\ifhlines \hlinesfalse

\newcommand{\verbistelines}{\vlinestrue\hlinestrue}
\newcommand{\verbistefontsize}[1]{\def\vbfs{#1}}
\newcommand{\verbistepackage}[2]{\AtEndOfClass{\RequirePackage[#1]{#2}}}

\verbistefontsize{12pt}

% read cfg file if it exist 

\InputIfFileExists{verbiste.cfg}{%
\typeout{Using configuration verbiste.cfg}
}{%
\typeout{No configuration file}
}

% load the article class with fontsize \vbfs

\LoadClass[\vbfs]{article}

% french language setup

\RequirePackage[utf8]{inputenc}
\RequirePackage[cyr]{aeguill}
\RequirePackage[french]{babel}

% handles the spacing above and below the tense name
% and below the persons of the tense

\newcommand\topstrut{\rule{0pt}{3.5ex}}
\newcommand\midstrut{\rule[-1ex]{0pt}{0pt}}
\newcommand\botstrut{\rule[-2ex]{0pt}{0pt}}

% the tense name

\newcommand{\tensename}[1]{%
  \textbf{#1}\topstrut\midstrut}

% the tense environment

\newenvironment{tense}{%
  \begin{tabular}[t]{l}}{\botstrut\end{tabular}}

% tense table horizontal lines

\newcommand{\vbhl}{%
  \ifhlines\hline\else\relax\fi}

% the tensetable environment

\newenvironment{tensetable}{%
  \begin{tabular}{%
      \ifvlines 
      |l|l|l|l|l|
      \else 
      llll
      \fi}\vbhl
}{\end{tabular}}

% fin

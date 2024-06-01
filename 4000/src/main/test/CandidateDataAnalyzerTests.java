import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;


public class CandidateDataAnalyzerTests {
    String emptySigns = "4000/data/TestDataEmptySigns";
    String data1 = "4000/data/TestData1";
    String application_1 = "4000/data/out/IBM-000123/Application_1";
    String application_2 = "4000/data/out/IBM-000123/Application_2";

    private static HashSet<String> createFilesSet(String... fileNames) {
        HashSet<String> filesSet = new HashSet<>();
        Collections.addAll(filesSet, fileNames);
        return filesSet;
    }

    @Test
    void testAnalysisForTestDataEmptySigns() {
        List<WordData> actual = CandidateDataAnalyzerMulti.CandidateDataAnalyzer(new String[]{emptySigns});
        List<WordData> expected = new ArrayList<>();
        assertEquals(expected.toString(), actual.toString());

    }
    @Test
    void testAnalysisForTestData1() {
        List<WordData> actual = CandidateDataAnalyzerMulti.CandidateDataAnalyzer(new String[]{data1});
        List<WordData> expected = new ArrayList<>();

        expected.add(new WordData("six", 6, createFilesSet("4000\\data\\TestData1\\6.txt")));
        expected.add(new WordData("five", 5, createFilesSet("4000\\data\\TestData1\\4 5.txt", "4000\\data\\TestData1\\2 5.txt")));
        expected.add(new WordData("four", 4, createFilesSet("4000\\data\\TestData1\\4 5.txt")));
        expected.add(new WordData("files", 4, createFilesSet("4000\\data\\TestData1\\1 2 3.txt", "4000\\data\\TestData1\\4 5.txt", "4000\\data\\TestData1\\6.txt", "4000\\data\\TestData1\\2 5.txt")));
        expected.add(new WordData("three", 3, createFilesSet("4000\\data\\TestData1\\1 2 3.txt")));
        expected.add(new WordData("two", 2, createFilesSet("4000\\data\\TestData1\\1 2 3.txt", "4000\\data\\TestData1\\2 5.txt")));
        expected.add(new WordData("one", 1, createFilesSet("4000\\data\\TestData1\\1 2 3.txt")));

        assertEquals(expected.toString(), actual.toString());

    }
    @Test
    void testAnalysisForApplication1Folder() {
        List<WordData> actual = CandidateDataAnalyzerMulti.CandidateDataAnalyzer(new String[]{application_1});
        List<WordData> expected = new ArrayList<>();
        expected.add(new WordData("the", 67960, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("of", 53397, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("and", 47972, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("to", 31147, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("in", 24765, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("a", 24646, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("is", 12920, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("for", 12870, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("km", 12711, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt")));
        expected.add(new WordData("with", 12047, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("rate", 10394, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt")));
        expected.add(new WordData("that", 9671, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("on", 9395, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("est", 9141, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt")));
        expected.add(new WordData("s", 9049, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-cv.txt")));
        expected.add(new WordData("by", 8812, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt")));
        expected.add(new WordData("or", 8650, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt",
                "4000\\data\\out\\IBM-000123\\Application_1\\1-report-1.txt")));
        expected.add(new WordData("us", 7947, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt")));
        expected.add(new WordData("it", 7649, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt")));
        expected.add(new WordData("total", 7145, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_1\\1-big-file-1.txt")));
        HashSet<String> sixFiles = new HashSet<>();
        assertEquals(expected.toString(), actual.toString());
    }

    @Test
    void testAnalysisForApplication2Folder() {
        List<WordData> actual = CandidateDataAnalyzerMulti.CandidateDataAnalyzer(new String[]{application_2});
        List<WordData> expected = new ArrayList<>();
        expected.add(new WordData("the", 118758, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("and", 76476, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("of", 66863, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("to", 58541, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("a", 45384, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("in", 35421, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("i", 33809, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("that", 29077, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("he", 28450, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("it", 25214, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("was", 23245, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("you", 20725, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("is", 17773, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("with", 17623, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("for", 17227, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-email.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("his", 17144, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("her", 16448, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("she", 15973, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("had", 15632, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-letter.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        expected.add(new WordData("as", 15370, createFilesSet(
                "4000\\data\\out\\IBM-000123\\Application_2\\2-cv.txt",
                "4000\\data\\out\\IBM-000123\\Application_2\\2-big-file1.txt")));

        assertEquals(expected.toString(), actual.toString());

    }

}

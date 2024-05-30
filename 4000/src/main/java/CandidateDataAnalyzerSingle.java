import java.io.*;
import java.util.*;
import java.util.concurrent.atomic.*;

public class CandidateDataAnalyzerSingle {
    private static final int TOP_N = 20;
    private static final Map<String, WordData> wordMap = new HashMap<>();

    // Synchronized method to update the shared word map
    private static synchronized void updateWordMap(String word, String fileName) {
        WordData wordData = wordMap.getOrDefault(word, new WordData(word));
        wordData.incrementCount();
        wordData.addFile(fileName);
        wordMap.put(word, wordData);
    }

    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("No files provided.");
            return;
        }

        Thread[] threads = new Thread[args.length];
        for (int i = 0; i < args.length; i++) {
            final String fileName = args[i];
            threads[i] = new Thread(() -> processFile(fileName));
            threads[i].start();
        }

        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        displayTopWords();
    }

    private static void processFile(String fileName) {
        try (BufferedReader reader = new BufferedReader(new FileReader(fileName))) {
            String line;
            while ((line = reader.readLine()) != null) {
                String[] words = line.split("\\W+");
                for (String word : words) {
                    if (!word.isEmpty()) {
                        updateWordMap(word.toLowerCase(), fileName);
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static void displayTopWords() {
        List<WordData> wordDataList = new ArrayList<>(wordMap.values());
        wordDataList.sort((wd1, wd2) -> Integer.compare(wd2.getCount(), wd1.getCount()));

        System.out.println("Top " + TOP_N + " most frequently referenced words:");
        for (int i = 0; i < Math.min(TOP_N, wordDataList.size()); i++) {
            WordData wordData = wordDataList.get(i);
            System.out.println(wordData);
        }
    }
}


import java.io.*;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

    public class CandidateDataAnalyzerMulti {
        private static final int TOP_N = 20;
        private static final int LINES_PER_THREAD = 100; // Number of lines each thread processes
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

            for (String fileName : args) {
                processFileWithMultipleThreads(fileName);
            }

            displayTopWords();
        }

        private static void processFileWithMultipleThreads(String fileName) {
            try (BufferedReader reader = new BufferedReader(new FileReader(fileName))) {
                List<String> lines = new ArrayList<>();
                String line;
                while ((line = reader.readLine()) != null) {
                    lines.add(line);
                }

                int numThreads = (lines.size() + LINES_PER_THREAD - 1) / LINES_PER_THREAD;
                List<Thread> threads = new ArrayList<>();

                for (int i = 0; i < numThreads; i++) {
                    final int start = i * LINES_PER_THREAD;
                    final int end = Math.min(start + LINES_PER_THREAD, lines.size());

                    Thread thread = new Thread(() -> {
                        for (int j = start; j < end; j++) {
                            processLine(lines.get(j), fileName);
                        }
                    });
                    threads.add(thread);
                    thread.start();
                }

                for (Thread thread : threads) {
                    try {
                        thread.join();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        private static void processLine(String line, String fileName) {
            String[] words = line.split("\\W+");
            for (String word : words) {
                if (!word.isEmpty()) {
                    updateWordMap(word.toLowerCase(), fileName);
                }
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
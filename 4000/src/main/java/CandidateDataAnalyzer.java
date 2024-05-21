import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.regex.*;

public class CandidateDataAnalyzer {
    private static final int TOP_N = 20;
    private static final Pattern WORD_PATTERN = Pattern.compile("\\b\\w+\\b");

    public static void main(String[] args) throws InterruptedException, ExecutionException {
        if (args.length == 0) {
            System.out.println("Please provide the paths to the candidate files.");
            return;
        }

        ExecutorService executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
        List<Future<Map<String, WordInfo>>> futures = new ArrayList<>();

        for (String filePath : args) {
            futures.add(executor.submit(() -> processFile(filePath)));
        }

        Map<String, WordInfo> combinedWordInfo = new HashMap<>();

        for (Future<Map<String, WordInfo>> future : futures) {
            Map<String, WordInfo> wordInfo = future.get();
            mergeWordInfo(combinedWordInfo, wordInfo);
        }

        executor.shutdown();

        List<WordInfo> sortedWordInfo = new ArrayList<>(combinedWordInfo.values());
        sortedWordInfo.sort((a, b) -> Integer.compare(b.count, a.count));

        System.out.println("Top " + TOP_N + " words:");
        for (int i = 0; i < Math.min(TOP_N, sortedWordInfo.size()); i++) {
            WordInfo wordInfo = sortedWordInfo.get(i);
            System.out.println(wordInfo.word + " - " + wordInfo.count + " occurrences");
            for (Map.Entry<String, List<Integer>> entry : wordInfo.locations.entrySet()) {
                System.out.println("  File: " + entry.getKey());
                System.out.println("    Lines: " + entry.getValue());
            }
        }
    }

    private static Map<String, WordInfo> processFile(String filePath) throws IOException {
        Map<String, WordInfo> wordInfoMap = new HashMap<>();
        List<String> lines = Files.readAllLines(Paths.get(filePath));

        for (int lineNumber = 0; lineNumber < lines.size(); lineNumber++) {
            Matcher matcher = WORD_PATTERN.matcher(lines.get(lineNumber));
            while (matcher.find()) {
                String word = matcher.group().toLowerCase();
                wordInfoMap.putIfAbsent(word, new WordInfo(word));
                WordInfo wordInfo = wordInfoMap.get(word);
                wordInfo.count++;
                wordInfo.locations.putIfAbsent(filePath, new ArrayList<>());
                wordInfo.locations.get(filePath).add(lineNumber + 1);
            }
        }

        return wordInfoMap;
    }

    private static void mergeWordInfo(Map<String, WordInfo> combined, Map<String, WordInfo> toMerge) {
        for (Map.Entry<String, WordInfo> entry : toMerge.entrySet()) {
            String word = entry.getKey();
            WordInfo newInfo = entry.getValue();

            combined.putIfAbsent(word, new WordInfo(word));
            WordInfo combinedInfo = combined.get(word);
            combinedInfo.count += newInfo.count;
            newInfo.locations.forEach((file, lines) -> {
                combinedInfo.locations.putIfAbsent(file, new ArrayList<>());
                combinedInfo.locations.get(file).addAll(lines);
            });
        }
    }

    static class WordInfo {
        String word;
        int count;
        Map<String, List<Integer>> locations;

        WordInfo(String word) {
            this.word = word;
            this.count = 0;
            this.locations = new HashMap<>();
        }
    }
}

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Other {
    private static final int TOP_N = 20;
    private static final Pattern WORD_PATTERN = Pattern.compile("\\b\\w+\\b");

    public static void main(String[] args) throws IOException, InterruptedException {
        if (args.length == 0) {
            System.out.println("Please provide the paths to the candidate files.");
        } else {
            List<Thread> threads = new ArrayList<>();
            Map<String, WordInfo> combinedWordInfo = new ConcurrentHashMap<>();

            for(String filePath : args) {
                Thread thread = new Thread(() -> {
                    try {
                        Map<String, WordInfo> wordInfo = processFile(filePath);
                        mergeWordInfo(combinedWordInfo, wordInfo);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                });
                threads.add(thread);
                thread.start();
            }

            for (Thread thread : threads) {
                thread.join();
            }

            List<WordInfo> sortedWordInfo = new ArrayList<>(combinedWordInfo.values());
            sortedWordInfo.sort((a, b) -> Integer.compare(b.count, a.count));
            System.out.println("Top 20 words:");

            for(int i = 0; i < Math.min(TOP_N, sortedWordInfo.size()); ++i) {
                WordInfo wordInfo = sortedWordInfo.get(i);
                System.out.println(wordInfo.word + " - " + wordInfo.count + " occurrences");

                for (Map.Entry<String, List<Integer>> entry : wordInfo.locations.entrySet()) {
                    System.out.println("  File: " + entry.getKey());
                    System.out.println("    Lines: " + entry.getValue());
                }
            }
        }
    }

    private static Map<String, WordInfo> processFile(String filePath) throws IOException {
        Map<String, WordInfo> wordInfoMap = new HashMap<>();
        List<String> lines = Files.readAllLines(Paths.get(filePath));

        for(int lineNumber = 0; lineNumber < lines.size(); ++lineNumber) {
            Matcher matcher = WORD_PATTERN.matcher(lines.get(lineNumber));

            while(matcher.find()) {
                String word = matcher.group().toLowerCase();
                wordInfoMap.putIfAbsent(word, new WordInfo(word));
                WordInfo wordInfo = wordInfoMap.get(word);
                ++wordInfo.count;
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
            combined.compute(word, (key, existing) -> {
                if (existing == null) {
                    return newInfo;
                } else {
                    existing.count += newInfo.count;
                    newInfo.locations.forEach((file, lines) -> {
                        existing.locations.putIfAbsent(file, new ArrayList<>());
                        existing.locations.get(file).addAll(lines);
                    });
                    return existing;
                }
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
